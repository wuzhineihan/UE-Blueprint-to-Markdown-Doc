/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Private/Trace/FMarkdownPathTracer.cpp
#include "Trace/FMarkdownPathTracer.h"

#include "EdGraph/EdGraph.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "UObject/UObjectIterator.h"
#include "K2Node_CallFunction.h"
#include "K2Node_MacroInstance.h"
#include "K2Node_Composite.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_Tunnel.h"
#include "UObject/UObjectBaseUtility.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Trace/MarkdownDataTracer.h"
#include "Formatter/FMarkdownNodeFormatter.h"
#include "Extractors/BlueprintDataExtractor.h"
#include "Logging/BP2AILog.h"
#include "Logging/LogMacros.h"
#include "Utils/MarkdownTracerUtils.h"
#include "Formatter/Formatters_Private.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Utils/MarkdownFormattingUtils.h" // For FMarkdownSpan
#include "UObject/UObjectGlobals.h"
#include "Trace/MarkdownGenerationContext.h" // Required for FMarkdownSpan::GetCurrentContext()
#include "Utils/MarkdownSpanSystem.h"

namespace {
    /**
     * Result of analyzing a function call for interface context
     * Determines if the call is to an interface definition or implementation
     */
    struct FInterfaceAnalysisResult
    {
        bool bIsInterfaceDefinition = false;      // Function is in a BPI (Blueprint Interface) asset
        bool bIsInterfaceImplementation = false;  // Function is interface implementation in regular BP
        FString InterfaceName;                    // Name of the interface class
        FString QualifiedHint;                    // "InterfaceName.FunctionName" format
        
        FInterfaceAnalysisResult() = default;
        
        bool IsInterface() const 
        { 
            return bIsInterfaceDefinition || bIsInterfaceImplementation; 
        }
    };
    
    /**
     * Extract function name from a GraphNameHint when UFunction is not available
     * Handles hints like "MyBP.MyFunction" -> "MyFunction"
     */
    FString ExtractFunctionNameFromHint(const FString& GraphNameHint)
    {
        FString FunctionName = GraphNameHint;
        
        // Remove blueprint name prefix if present
        int32 LastDotIndex = -1;
        if (GraphNameHint.FindLastChar(TEXT('.'), LastDotIndex))
        {
            FunctionName = GraphNameHint.Mid(LastDotIndex + 1);
        }
        
        // Clean up any remaining prefixes or suffixes
        FunctionName = FunctionName.TrimStartAndEnd();
        
        return FunctionName;
    }
    
    /**
     * Analyze a function call to determine if it's interface-related
     * Handles both BPI definitions and interface implementations in regular BPs
     */
    FInterfaceAnalysisResult AnalyzeForInterfaceContext(
        UBlueprint* OwningBP, 
        UFunction* Function, 
        const FString& OriginalHint)
    {
        FInterfaceAnalysisResult Result;
        
        if (!OwningBP)
        {
            UE_LOG(LogPathTracer, Verbose, TEXT("AnalyzeForInterfaceContext: No owning BP provided"));
            return Result;
        }
        
        // CASE 1: BPI Definition - Blueprint is itself an interface
        if (OwningBP->BlueprintType == BPTYPE_Interface)
        {
            Result.bIsInterfaceDefinition = true;
            Result.InterfaceName = OwningBP->GetName();
            
            // Remove common suffixes from interface name
            if (Result.InterfaceName.EndsWith(TEXT("_C")))
            {
                Result.InterfaceName.LeftChopInline(2);
            }
            Result.InterfaceName.RemoveFromStart(TEXT("Default__"));
            
            // Extract function name from UFunction or hint
            FString FunctionName;
            if (Function)
            {
                FunctionName = Function->GetName();
            }
            else
            {
                FunctionName = ExtractFunctionNameFromHint(OriginalHint);
            }
            
            // Generate qualified hint
            Result.QualifiedHint = FString::Printf(TEXT("%s.%s"), *Result.InterfaceName, *FunctionName);
            
            UE_LOG(LogPathTracer, Warning, TEXT("AnalyzeForInterfaceContext: BPI Definition detected - Interface: %s, Function: %s, QualifiedHint: %s"),
                   *Result.InterfaceName, *FunctionName, *Result.QualifiedHint);
            
            return Result;
        }
        
        // CASE 2: Interface Implementation - Regular BP implementing interface function
        if (Function && OwningBP->BlueprintType != BPTYPE_Interface)
        {
            // Check all implemented interfaces
            for (const FBPInterfaceDescription& InterfaceDesc : OwningBP->ImplementedInterfaces)
            {
                if (UClass* InterfaceClass = InterfaceDesc.Interface)
                {
                    // Check if this interface defines the function we're looking for
                    UFunction* InterfaceFunction = InterfaceClass->FindFunctionByName(Function->GetFName());
                    if (InterfaceFunction)
                    {
                        Result.bIsInterfaceImplementation = true;
                        Result.InterfaceName = InterfaceClass->GetName();
                        
                        // Clean interface class name
                        if (Result.InterfaceName.EndsWith(TEXT("_C")))
                        {
                            Result.InterfaceName.LeftChopInline(2);
                        }
                        Result.InterfaceName.RemoveFromStart(TEXT("Default__"));
                        
                        // Generate qualified hint using interface name
                        Result.QualifiedHint = FString::Printf(TEXT("%s.%s"), *Result.InterfaceName, *Function->GetName());
                        
                        UE_LOG(LogPathTracer, Warning, TEXT("AnalyzeForInterfaceContext: Interface Implementation detected - Interface: %s, Function: %s, QualifiedHint: %s, ImplementingBP: %s"),
                               *Result.InterfaceName, *Function->GetName(), *Result.QualifiedHint, *OwningBP->GetName());
                        
                        return Result; // Return first match
                    }
                }
            }
        }
        
        // CASE 3: Not an interface call
        UE_LOG(LogPathTracer, Verbose, TEXT("AnalyzeForInterfaceContext: Not an interface call - BP: %s, Function: %s"),
               OwningBP ? *OwningBP->GetName() : TEXT("NULL"), Function ? *Function->GetName() : TEXT("NULL"));
        
        return Result;
    }
}


FString FMarkdownPathTracer::GenerateMarkdownLine(const FString& Content, const FString& IndentPrefix) const
{
    return IndentPrefix + ExecPrefix + Content; // ExecPrefix is a member, accessible now
}
// --- End Helper Implementations ---


// --- Static Private Helper Function Definitions (Unchanged from your provided file) ---
FMarkdownPathTracer::EUserGraphType FMarkdownPathTracer::CheckMacroInstanceType_Helper(
    TSharedPtr<const FBlueprintNode> Node,
    const FString& CallingBlueprintName,
    FString& OutGraphPath,
    FString& OutGraphNameHint)
{
    const FString* MacroGraphReferencePathPtr = Node->RawProperties.Find(TEXT("MacroGraphReference"));
    if (MacroGraphReferencePathPtr && !MacroGraphReferencePathPtr->IsEmpty())
    {
        UE_LOG(LogPathTracer, Log, TEXT("  Helper_CheckMacroInstanceType: Found MacroGraphReference Path: '%s'"), **MacroGraphReferencePathPtr);
        bool bIsStandardEngineMacro = MacroGraphReferencePathPtr->Contains(TEXT("/Engine/EditorBlueprintResources/StandardMacros")) ||
                                        MacroGraphReferencePathPtr->Contains(TEXT("/Engine/EditorLandscapeResources/DefaultMacros"));
        if (!bIsStandardEngineMacro)
        {
            UEdGraph* MacroGraph = FindObject<UEdGraph>(nullptr, **MacroGraphReferencePathPtr);
            if (MacroGraph) {
                OutGraphPath = MacroGraph->GetPathName();
                UBlueprint* MacroOwningBP = FBlueprintEditorUtils::FindBlueprintForGraph(MacroGraph);
                FString MacroSimpleName = Node->Name; 
                if (MacroSimpleName.IsEmpty() || MacroSimpleName == Node->NodeType) { 
                    MacroSimpleName = FPaths::GetCleanFilename(OutGraphPath);
                    if (MacroSimpleName.EndsWith(TEXT("_Graph"))) MacroSimpleName.LeftChopInline(6);
                }

                if (MacroOwningBP) {
                    FString BAssetName = MacroOwningBP->GetName();
                    BAssetName.RemoveFromStart(TEXT("Default__"));
                    if (BAssetName.EndsWith(TEXT("_C"))) BAssetName.LeftChopInline(2);
                    OutGraphNameHint = FString::Printf(TEXT("%s.%s"), *BAssetName, *MacroSimpleName);
                } else {
                    OutGraphNameHint = MacroSimpleName;
                    UE_LOG(LogPathTracer, Verbose, TEXT("    Helper_CheckMacroInstanceType: User Macro '%s' - MacroOwningBP not found. Using simple name for hint."), *MacroSimpleName);
                }
                UE_LOG(LogPathTracer, Log, TEXT("    Helper_CheckMacroInstanceType: User Macro. Path: '%s', Hint: '%s', Type: Macro."), *OutGraphPath, *OutGraphNameHint);
                return FMarkdownPathTracer::EUserGraphType::Macro;
            } else {
                    UE_LOG(LogPathTracer, Warning, TEXT("    Helper_CheckMacroInstanceType: User Macro. MacroGraph object NOT FOUND via path: '%s'."), **MacroGraphReferencePathPtr);
            }
        } else {
            UE_LOG(LogPathTracer, Log, TEXT("    Helper_CheckMacroInstanceType: Standard Engine Macro. Path: '%s'. Not queued for definition."), **MacroGraphReferencePathPtr);
        }
    } else {
            UE_LOG(LogPathTracer, Warning, TEXT("  Helper_CheckMacroInstanceType: Node '%s' missing MacroGraphReference property."), *Node->Name);
    }
    return FMarkdownPathTracer::EUserGraphType::Unknown;
}

FMarkdownPathTracer::EUserGraphType FMarkdownPathTracer::CheckCompositeNodeType_Helper(
    TSharedPtr<const FBlueprintNode> Node,
    const FString& CallingBlueprintName,
    FString& OutGraphPath,
    FString& OutGraphNameHint)
{
    const FString* BoundGraphPathPtr = Node->RawProperties.Find(TEXT("BoundGraphName"));
    if (BoundGraphPathPtr && !BoundGraphPathPtr->IsEmpty())
    {
        UE_LOG(LogPathTracer, Log, TEXT("  Helper_CheckCompositeNodeType: Found BoundGraphName Path: '%s'"), **BoundGraphPathPtr);
        UEdGraph* BoundGraph = FindObject<UEdGraph>(nullptr, **BoundGraphPathPtr);
        if (BoundGraph) {
            OutGraphPath = BoundGraph->GetPathName();
            FString CompositeInstanceName = Node->Name.Replace(TEXT("\n"), TEXT(" "));
            if (CompositeInstanceName.IsEmpty() || CompositeInstanceName == Node->NodeType.Replace(TEXT("\n"), TEXT(" "))) {
                    CompositeInstanceName = FPaths::GetBaseFilename(OutGraphPath);
                    if (CompositeInstanceName.EndsWith(TEXT("_Graph"))) CompositeInstanceName.LeftChopInline(6);
            }

            UBlueprint* CompositeOwningBP = FBlueprintEditorUtils::FindBlueprintForGraph(BoundGraph);
            if (CompositeOwningBP) {
                FString BAssetName = CompositeOwningBP->GetName();
                BAssetName.RemoveFromStart(TEXT("Default__"));
                if (BAssetName.EndsWith(TEXT("_C"))) BAssetName.LeftChopInline(2);
                OutGraphNameHint = FString::Printf(TEXT("%s.%s"), *BAssetName, *CompositeInstanceName);
            } else {
                OutGraphNameHint = CompositeInstanceName;
                if(OutGraphNameHint.IsEmpty()) OutGraphNameHint = FPaths::GetCleanFilename(OutGraphPath); 
                UE_LOG(LogPathTracer, Warning, TEXT("    Helper_CheckCompositeNodeType: CompositeOwningBP not found for BoundGraph '%s'. Using instance/graph name for hint."), *BoundGraph->GetName());
            }
            UE_LOG(LogPathTracer, Log, TEXT("    Helper_CheckCompositeNodeType: Collapsed Graph. Path: '%s', Hint: '%s', Type: CollapsedGraph."), *OutGraphPath, *OutGraphNameHint);
            return FMarkdownPathTracer::EUserGraphType::CollapsedGraph;
        } else {
            UE_LOG(LogPathTracer, Warning, TEXT("    Helper_CheckCompositeNodeType: BoundGraph object NOT FOUND via path: '%s'."), **BoundGraphPathPtr);
        }
    } else {
        UE_LOG(LogPathTracer, Warning, TEXT("  Helper_CheckCompositeNodeType: Node '%s' missing BoundGraphName property."), *Node->Name);
    }
    return FMarkdownPathTracer::EUserGraphType::Unknown;
}

FMarkdownPathTracer::EUserGraphType FMarkdownPathTracer::CheckCallFunctionType_Helper(
    TSharedPtr<const FBlueprintNode> Node,
    UBlueprint* CallingNodeOwningBlueprint,
    const FString& CallingBlueprintName,
    FString& OutGraphPath,
    FString& OutGraphNameHint)
{
    UEdGraphNode* EdGraphNode = Node->GetEdGraphNode(); 
    UK2Node_CallFunction* K2CallFuncNode = Cast<UK2Node_CallFunction>(EdGraphNode);
    if (!K2CallFuncNode) {
        UE_LOG(LogPathTracer, Error, TEXT("  Helper_CheckCallFunctionType: Cast to UK2Node_CallFunction failed for Node '%s' (Type: %s)."), *Node->Name, *Node->NodeType);
        return FMarkdownPathTracer::EUserGraphType::Unknown;
    }

    const FMemberReference& ResolvedFunctionReference = K2CallFuncNode->FunctionReference;
    FName TargetFunctionName = ResolvedFunctionReference.GetMemberName();
    UClass* CallingContextClassForResolve = CallingNodeOwningBlueprint ? CallingNodeOwningBlueprint->GeneratedClass : nullptr;
    UClass* TargetFunctionOwnerClass = ResolvedFunctionReference.GetMemberParentClass(CallingContextClassForResolve);

    UE_LOG(LogPathTracer, Log, TEXT("  Helper_CheckCallFunctionType: Node Title='%s', TargetFunctionName='%s'."), *K2CallFuncNode->GetNodeTitle(ENodeTitleType::ListView).ToString(), *TargetFunctionName.ToString());

    if (TargetFunctionName == NAME_None) {
        UE_LOG(LogPathTracer, Warning, TEXT("    Helper_CheckCallFunctionType: TargetFunctionName is NAME_None for node '%s'. Returning Unknown."), *Node->Name);
        return FMarkdownPathTracer::EUserGraphType::Unknown;
    }
    if (!TargetFunctionOwnerClass) {
        const FString* ParentClassPathFromProp = Node->RawProperties.Find(TEXT("FunctionParentClassPath"));
        if(ParentClassPathFromProp && !ParentClassPathFromProp->IsEmpty()) {
            TargetFunctionOwnerClass = FindObject<UClass>(nullptr, **ParentClassPathFromProp);
            UE_LOG(LogPathTracer, Log, TEXT("    Helper_CheckCallFunctionType: Resolved TargetFunctionOwnerClass from RawProperty 'FunctionParentClassPath' ('%s') to: %s"), **ParentClassPathFromProp, TargetFunctionOwnerClass ? *TargetFunctionOwnerClass->GetPathName() : TEXT("NULL"));
        }
        if (!TargetFunctionOwnerClass) {
                UE_LOG(LogPathTracer, Warning, TEXT("    Helper_CheckCallFunctionType: TargetFunctionOwnerClass is NULL (even after checking RawProperties) for node '%s', TargetFunc '%s'. Returning Unknown."), *Node->Name, *TargetFunctionName.ToString());
                return FMarkdownPathTracer::EUserGraphType::Unknown;
        }
    }
    UE_LOG(LogPathTracer, Log, TEXT("    Helper_CheckCallFunctionType: TargetFunctionOwnerClass (after potential prop lookup): '%s'"), *TargetFunctionOwnerClass->GetPathName());

    UFunction* TargetUFunction = TargetFunctionOwnerClass->FindFunctionByName(TargetFunctionName);
    UE_LOG(LogPathTracer, Log, TEXT("    Helper_CheckCallFunctionType: TargetUFunction ('%s' in '%s') resolved to: %s"),
        *TargetFunctionName.ToString(), *TargetFunctionOwnerClass->GetName(), TargetUFunction ? *TargetUFunction->GetPathName() : TEXT("NULL"));

    UObject* FunctionActualOuter = TargetUFunction ? TargetUFunction->GetOuter() : nullptr;
    UBlueprint* FunctionDefiningBP = Cast<UBlueprint>(FunctionActualOuter);
    if (!FunctionDefiningBP && FunctionActualOuter && FunctionActualOuter->IsA<UBlueprintGeneratedClass>()) {
        FunctionDefiningBP = Cast<UBlueprint>(Cast<UBlueprintGeneratedClass>(FunctionActualOuter)->ClassGeneratedBy);
    }
    UE_LOG(LogPathTracer, Log, TEXT("    Helper_CheckCallFunctionType: FunctionDefiningBP (for '%s'): %s"),
        TargetUFunction ? *TargetUFunction->GetName() : *TargetFunctionName.ToString(),
        FunctionDefiningBP ? *FunctionDefiningBP->GetPathName() : TEXT("NULL - Likely Native/C++ or Interface Definition/Not in BP"));

    if (!FunctionDefiningBP) {
        FString OwnerClassNameForDisplay = TargetFunctionOwnerClass->GetName();
        FString FullOwnerClassPath = TargetFunctionOwnerClass->GetPathName();
        if (FullOwnerClassPath.StartsWith(TEXT("/Script/"))) {
            FString ModuleName = FullOwnerClassPath;
            ModuleName.RemoveFromStart(TEXT("/Script/"));
            int32 DotPos;
            if (ModuleName.FindChar(TEXT('.'), DotPos)) {
                ModuleName = ModuleName.Left(DotPos);
            }
            OwnerClassNameForDisplay = ModuleName + TEXT(".") + TargetFunctionOwnerClass->GetName();
        } else if (FullOwnerClassPath.StartsWith(TEXT("/Engine/"))) {
                if (TargetFunctionOwnerClass == UKismetSystemLibrary::StaticClass()) OwnerClassNameForDisplay = TEXT("KismetSystemLibrary");
                else if (TargetFunctionOwnerClass == UKismetMathLibrary::StaticClass()) OwnerClassNameForDisplay = TEXT("KismetMathLibrary");
                else if (TargetFunctionOwnerClass == UKismetStringLibrary::StaticClass()) OwnerClassNameForDisplay = TEXT("KismetStringLibrary");
                else if (TargetFunctionOwnerClass == UGameplayStatics::StaticClass()) OwnerClassNameForDisplay = TEXT("GameplayStatics");
            else OwnerClassNameForDisplay = TEXT("Engine.") + TargetFunctionOwnerClass->GetName();
        } else {
            OwnerClassNameForDisplay = MarkdownTracerUtils::ExtractSimpleNameFromPath(FullOwnerClassPath, TEXT(""));
        }
        OutGraphNameHint = FString::Printf(TEXT("%s.%s"), *OwnerClassNameForDisplay, *TargetFunctionName.ToString());
        UE_LOG(LogPathTracer, Log, TEXT("    Helper_CheckCallFunctionType: Call to NATIVE/C++ or Interface Definition. Hint: '%s'. Not queued for separate BP graph definition."), *OutGraphNameHint);
        return FMarkdownPathTracer::EUserGraphType::Unknown;
    }
    else
    {
        // === NEW INTERFACE ANALYSIS ===
        FInterfaceAnalysisResult InterfaceResult = AnalyzeForInterfaceContext(FunctionDefiningBP, TargetUFunction, TargetFunctionName.ToString());
    
        if (InterfaceResult.IsInterface())
        {
            // Interface detected - use qualified hint and interface-specific path resolution
            UEdGraph* TargetEdGraphFromScope = FBlueprintEditorUtils::FindScopeGraph(FunctionDefiningBP, TargetUFunction);
        
            if (TargetEdGraphFromScope)
            {
                OutGraphPath = TargetEdGraphFromScope->GetPathName();
                OutGraphNameHint = InterfaceResult.QualifiedHint;  // Use qualified hint like "IMyInterface.DoSomething"
            
                UE_LOG(LogPathTracer, Error, TEXT("CheckCallFunctionType_Helper: INTERFACE DETECTED - Type: %s, QualifiedHint: %s, Path: %s"),
                       InterfaceResult.bIsInterfaceDefinition ? TEXT("BPI Definition") : TEXT("Implementation"),
                       *InterfaceResult.QualifiedHint,
                       *OutGraphPath);
            
                return FMarkdownPathTracer::EUserGraphType::Interface; // Will be re-categorized as Interface in Phase 1.5
            }
            else
            {
                UE_LOG(LogPathTracer, Warning, TEXT("CheckCallFunctionType_Helper: Interface detected but FindScopeGraph failed for %s"), *InterfaceResult.QualifiedHint);
                return FMarkdownPathTracer::EUserGraphType::Unknown;
            }
        }
        // === END INTERFACE ANALYSIS ===

        
        UEdGraph* FoundCustomEventHostGraph = nullptr;
        bool bIdentifiedAsCustomEvent = false;

        for (UEdGraph* GraphPage : FunctionDefiningBP->UbergraphPages)
        {
            if (!GraphPage) continue;
            for (UEdGraphNode* CurrentGraphNode_Inner : GraphPage->Nodes)
            {
                if (UK2Node_CustomEvent* CustomEventNode = Cast<UK2Node_CustomEvent>(CurrentGraphNode_Inner))
                {
                    if (CustomEventNode->CustomFunctionName == TargetFunctionName)
                    {
                        FoundCustomEventHostGraph = GraphPage;
                        bIdentifiedAsCustomEvent = true;
                        UE_LOG(LogPathTracer, Warning, TEXT("  >>>> Helper_CheckCallFunctionType: DIRECT CE MATCH! Found Custom Event '%s' on UbergraphPage '%s' (Path: %s)"),
                                *TargetFunctionName.ToString(), *GraphPage->GetName(), *GraphPage->GetPathName());
                        break;
                    }
                }
            }
            if (bIdentifiedAsCustomEvent) break;
        }

        if (!bIdentifiedAsCustomEvent)
        {
            for (UEdGraph* FuncGraphPage : FunctionDefiningBP->FunctionGraphs)
            {
                if (!FuncGraphPage) continue;
                for (UEdGraphNode* CurrentGraphNode_Inner : FuncGraphPage->Nodes) 
                {
                    if (UK2Node_CustomEvent* CustomEventNode = Cast<UK2Node_CustomEvent>(CurrentGraphNode_Inner))
                    {
                        if (CustomEventNode->CustomFunctionName == TargetFunctionName)
                        {
                            FoundCustomEventHostGraph = FuncGraphPage;
                            bIdentifiedAsCustomEvent = true;
                            UE_LOG(LogPathTracer, Warning, TEXT("  >>>> Helper_CheckCallFunctionType: DIRECT CE MATCH! Found Custom Event '%s' on FunctionGraph '%s' (Path: %s) - Unusual Placement."),
                                    *TargetFunctionName.ToString(), *FuncGraphPage->GetName(), *FuncGraphPage->GetPathName());
                            break;
                        }
                    }
                }
                if (bIdentifiedAsCustomEvent) break;
            }
        }

        if (bIdentifiedAsCustomEvent && FoundCustomEventHostGraph)
        {
            OutGraphPath = FoundCustomEventHostGraph->GetPathName();
            FString BAssetName = FunctionDefiningBP->GetName();
            BAssetName.RemoveFromStart(TEXT("Default__"));
            if (BAssetName.EndsWith(TEXT("_C"))) BAssetName.LeftChopInline(2);
            OutGraphNameHint = FString::Printf(TEXT("%s.%s"), *BAssetName, *TargetFunctionName.ToString());
            UE_LOG(LogPathTracer, Error, TEXT("Helper_CheckCallFunctionType: SUCCESS - Custom Event '%s' CORRECTLY IDENTIFIED. Path: '%s', Hint: '%s', Type: CustomEventGraph"),
                    *TargetFunctionName.ToString(), *OutGraphPath, *OutGraphNameHint);
            return FMarkdownPathTracer::EUserGraphType::CustomEventGraph;
        }
        else
        {
            UE_LOG(LogPathTracer, Error, TEXT("  >>>> Helper_CheckCallFunctionType: About to call FindScopeGraph (Non-Interface Path) as CE direct search FAILED or was not applicable."));
            UEdGraph* TargetEdGraphFromScope = FBlueprintEditorUtils::FindScopeGraph(FunctionDefiningBP, TargetUFunction);
            UE_LOG(LogPathTracer, Error, TEXT("  <<<< Helper_CheckCallFunctionType: FindScopeGraph (Non-Interface Path) returned UEdGraph*: Name='%s', Path='%s'"),
                TargetEdGraphFromScope ? *TargetEdGraphFromScope->GetName() : TEXT("NULL_GRAPH_RETURNED"),
                TargetEdGraphFromScope ? *TargetEdGraphFromScope->GetPathName() : TEXT("NULL_GRAPH_PATH_RETURNED"));

            if (TargetEdGraphFromScope)
            {
                OutGraphPath = TargetEdGraphFromScope->GetPathName();
                FString BAssetName = FunctionDefiningBP->GetName();
                BAssetName.RemoveFromStart(TEXT("Default__"));
                if (BAssetName.EndsWith(TEXT("_C"))) BAssetName.LeftChopInline(2);
                OutGraphNameHint = FString::Printf(TEXT("%s.%s"), *BAssetName, *TargetFunctionName.ToString());
                UE_LOG(LogPathTracer, Log, TEXT("    Helper_CheckCallFunctionType: Fallback to FindScopeGraph - Assuming Function. Path: '%s', Hint: '%s'."), *OutGraphPath, *OutGraphNameHint);
                return FMarkdownPathTracer::EUserGraphType::Function;
            }
            else
            {
                UE_LOG(LogPathTracer, Warning, TEXT("    Helper_CheckCallFunctionType: Fallback to FindScopeGraph also failed for function/event '%s' in BP '%s'."),
                        *TargetFunctionName.ToString(), *FunctionDefiningBP->GetName());
                return FMarkdownPathTracer::EUserGraphType::Unknown;
            }
        }
    }
    return FMarkdownPathTracer::EUserGraphType::Unknown;
}

// Static Helper
bool FMarkdownPathTracer::ProcessUserFunctionCall_Helper(
    FMarkdownPathTracer* Self,
    TSharedPtr<const FBlueprintNode> ExecutableNode, const FString& ActualGraphPath, const FString& UniqueGraphNameHint,
    const FString& DisplayTargetPrefix, const FString& FinalLinkTextForDisplay, const FString& ArgsStr,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& InOutProcessedGlobally,
    const FString& CurrentIndentPrefix, bool bIsLastSegment, TArray<FString>& OutLines,
    bool bWasAlreadyGloballyProcessed, TSharedPtr<const FBlueprintNode>& OutNextNodeToTrace)
{
    OutNextNodeToTrace = nullptr;
    FString AnchorName = FMarkdownPathTracer::SanitizeAnchorName(UniqueGraphNameHint);
    
    // CRITICAL FIX: Generate format based on current context
    FString LineContent;
    
    // Check if we can determine HTML context (simple heuristic)
    bool bIsHTMLMode = (Self->ExecPrefix.Contains(TEXT("<")) || Self->ExecPrefix.Contains(TEXT("&"))) || 
                       FMarkdownSpanSystem::GetCurrentContext().IsHTML();
    
    if (bIsHTMLMode) {
        // Generate HTML format with proper CSS classes and anchor links
        FString CallTypeClass = (ExecutableNode->NodeType == TEXT("CallParentFunction")) ? 
            TEXT("bp-call-parent-function") : TEXT("bp-call-function");
        FString LinkClass = TEXT("function-link");
        
        FString CallKeywordHtml = FString::Printf(TEXT("<span class=\"%s\">%s</span>"), 
            *CallTypeClass, 
            ExecutableNode->NodeType == TEXT("CallParentFunction") ? TEXT("Call Parent") : TEXT("Call Function"));
        FString LinkHtml = FString::Printf(TEXT("<a href=\"#%s\" class=\"graph-link %s\">%s</a>"), 
            *AnchorName, *LinkClass, *FMarkdownSpanSystem::EscapeHtml(FinalLinkTextForDisplay));
        
        LineContent = DisplayTargetPrefix + CallKeywordHtml + TEXT(": ") + LinkHtml + ArgsStr;
        
        if (bWasAlreadyGloballyProcessed) {
            LineContent += TEXT(" <span class=\"repeat-indicator\">(Call site repeat or graph already defined/queued)</span>");
        }
    } else {
        // Generate Markdown format (original logic)
        FString NodeTypeKeywordDisplay = (ExecutableNode->NodeType == TEXT("CallParentFunction")) ? 
            TEXT("Call Parent") : TEXT("Call Function");
        FString BaseLinkText = FString::Printf(TEXT("%s%s: [%s](#%s)"), 
            *DisplayTargetPrefix, *NodeTypeKeywordDisplay, *FinalLinkTextForDisplay, *AnchorName);
        LineContent = BaseLinkText + ArgsStr;
        
        if (bWasAlreadyGloballyProcessed) {
            LineContent += TEXT(" (Call site repeat or graph already defined/queued)");
        }
    }

    OutLines.Add(Self->GenerateMarkdownLine(LineContent, CurrentIndentPrefix));
    
    if (!bWasAlreadyGloballyProcessed && Self->bCurrentDefineUserGraphsSeparately && Self->CurrentGraphsToDefineSeparatelyPtr && Self->CurrentProcessedSeparateGraphPathsPtr) {
        if (!Self->CurrentProcessedSeparateGraphPathsPtr->Contains(ActualGraphPath)) {
            UE_LOG(LogPathTracer, Error, TEXT("ProcessUserFunctionCall_Helper: QUEUING for definition: Hint='%s', Path='%s', Type=Function"), *UniqueGraphNameHint, *ActualGraphPath);
            Self->CurrentGraphsToDefineSeparatelyPtr->AddUnique(MakeTuple(UniqueGraphNameHint, ActualGraphPath, FMarkdownPathTracer::EUserGraphType::Function));
        }
    }

    TSharedPtr<FBlueprintPin> OutputExecPin = ExecutableNode->GetExecutionOutputPin();
    if (OutputExecPin && OutputExecPin->LinkedPins.Num() > 0 && OutputExecPin->LinkedPins[0].IsValid()) {
        TSharedPtr<FBlueprintPin> TargetPinOnNext = OutputExecPin->LinkedPins[0];
        const TSharedPtr<FBlueprintNode>* NextNodeInCallerGraphPtr = AllNodes.Find(TargetPinOnNext->NodeGuid);
        if (NextNodeInCallerGraphPtr && NextNodeInCallerGraphPtr->IsValid()) {
            OutNextNodeToTrace = *NextNodeInCallerGraphPtr;
        } else { 
            if (!bWasAlreadyGloballyProcessed) {
                FString EndMessage = FString::Printf(TEXT("[Path ends after call to function '%s' - Next node not in selection]"), *FinalLinkTextForDisplay);
                OutLines.Add(Self->GenerateMarkdownLine(EndMessage, Self->CalculateNextPrefix(CurrentIndentPrefix, bIsLastSegment)));
            }
        }
    } else { 
        if (!bWasAlreadyGloballyProcessed) {
            FString EndMessage = FString::Printf(TEXT("[Path ends after call to function '%s']"), *FinalLinkTextForDisplay);
            OutLines.Add(Self->GenerateMarkdownLine(EndMessage, Self->CalculateNextPrefix(CurrentIndentPrefix, bIsLastSegment)));
        }
    }
    return true;
}

bool FMarkdownPathTracer::ProcessMacroCall_Helper(
    FMarkdownPathTracer* Self,
    TSharedPtr<const FBlueprintNode> ExecutableNode, const FString& ActualGraphPath, const FString& UniqueGraphNameHint,
    const FString& DisplayTargetPrefix, const FString& FinalLinkTextForDisplay, const FString& ArgsStr,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& InOutProcessedGlobally,
    const FString& CurrentIndentPrefix, bool bIsLastSegment, TArray<FString>& OutLines,
    bool bWasAlreadyGloballyProcessed, TSharedPtr<const FBlueprintNode>& OutNextNodeToTrace)
{
    OutNextNodeToTrace = nullptr;
    FString AnchorName = FMarkdownPathTracer::SanitizeAnchorName(UniqueGraphNameHint);
    
    // CRITICAL FIX: Generate format based on current context
    FString LineContent;
    
    bool bIsHTMLMode = (Self->ExecPrefix.Contains(TEXT("<")) || Self->ExecPrefix.Contains(TEXT("&"))) || 
                       FMarkdownSpanSystem::GetCurrentContext().IsHTML();
    
    if (bIsHTMLMode) {
        // Generate HTML format with proper CSS classes and anchor links
        FString CallTypeClass = TEXT("bp-call-macro");
        FString LinkClass = TEXT("macro-link");
        
        FString CallKeywordHtml = FString::Printf(TEXT("<span class=\"%s\">Macro</span>"), *CallTypeClass);
        FString LinkHtml = FString::Printf(TEXT("<a href=\"#%s\" class=\"graph-link %s\">%s</a>"), 
            *AnchorName, *LinkClass, *FMarkdownSpanSystem::EscapeHtml(FinalLinkTextForDisplay));
        
        LineContent = DisplayTargetPrefix + CallKeywordHtml + TEXT(": ") + LinkHtml + ArgsStr;
        
        if (bWasAlreadyGloballyProcessed) {
            LineContent += TEXT(" <span class=\"repeat-indicator\">(Call site repeat or graph already defined/queued)</span>");
        }
    } else {
        // Generate Markdown format (original logic)
        FString NodeTypeKeywordDisplay = TEXT("Macro");
        FString BaseLinkText = FString::Printf(TEXT("%s%s: [%s](#%s)"), 
            *DisplayTargetPrefix, *NodeTypeKeywordDisplay, *FinalLinkTextForDisplay, *AnchorName);
        LineContent = BaseLinkText + ArgsStr;
        
        if (bWasAlreadyGloballyProcessed) {
            LineContent += TEXT(" (Call site repeat or graph already defined/queued)");
        }
    }

    OutLines.Add(Self->GenerateMarkdownLine(LineContent, CurrentIndentPrefix));

    if (!bWasAlreadyGloballyProcessed && Self->bCurrentDefineUserGraphsSeparately && Self->CurrentGraphsToDefineSeparatelyPtr && Self->CurrentProcessedSeparateGraphPathsPtr) {
        if (!Self->CurrentProcessedSeparateGraphPathsPtr->Contains(ActualGraphPath)) {
             Self->CurrentGraphsToDefineSeparatelyPtr->AddUnique(MakeTuple(UniqueGraphNameHint, ActualGraphPath, FMarkdownPathTracer::EUserGraphType::Macro));
        }
    }
    
    TSharedPtr<FBlueprintPin> OutputExecPin = ExecutableNode->GetExecutionOutputPin();
    if (OutputExecPin && OutputExecPin->LinkedPins.Num() > 0 && OutputExecPin->LinkedPins[0].IsValid()) {
        TSharedPtr<FBlueprintPin> TargetPinOnNext = OutputExecPin->LinkedPins[0];
        const TSharedPtr<FBlueprintNode>* NextNodeInCallerGraphPtr = AllNodes.Find(TargetPinOnNext->NodeGuid);
        if (NextNodeInCallerGraphPtr && NextNodeInCallerGraphPtr->IsValid()) {
            OutNextNodeToTrace = *NextNodeInCallerGraphPtr;
        } else {
            if (!bWasAlreadyGloballyProcessed) {
                FString EndMessage = FString::Printf(TEXT("[Path ends after call to macro '%s' - Next node not in selection]"), *FinalLinkTextForDisplay);
                OutLines.Add(Self->GenerateMarkdownLine(EndMessage, Self->CalculateNextPrefix(CurrentIndentPrefix, bIsLastSegment)));
            }
        }
    } else {
        if (!bWasAlreadyGloballyProcessed) {
            FString EndMessage = FString::Printf(TEXT("[Path ends after call to macro '%s']"), *FinalLinkTextForDisplay);
            OutLines.Add(Self->GenerateMarkdownLine(EndMessage, Self->CalculateNextPrefix(CurrentIndentPrefix, bIsLastSegment)));
        }
    }
    return true;
}

bool FMarkdownPathTracer::ProcessCustomEventCall_Helper(
    FMarkdownPathTracer* Self,
    TSharedPtr<const FBlueprintNode> ExecutableNode, const FString& ActualGraphPath, const FString& UniqueGraphNameHint,
    const FString& DisplayTargetPrefix, const FString& FinalLinkTextForDisplay, const FString& ArgsStr,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& InOutProcessedGlobally,
    const FString& CurrentIndentPrefix, bool bIsLastSegment, TArray<FString>& OutLines,
    bool bWasAlreadyGloballyProcessed, TSharedPtr<const FBlueprintNode>& OutNextNodeToTrace)
{
    OutNextNodeToTrace = nullptr;
    FString AnchorName = FMarkdownPathTracer::SanitizeAnchorName(UniqueGraphNameHint);
    
    // CRITICAL FIX: Generate format based on current context
    FString LineContent;
    
    bool bIsHTMLMode = (Self->ExecPrefix.Contains(TEXT("<")) || Self->ExecPrefix.Contains(TEXT("&"))) || 
                       FMarkdownSpanSystem::GetCurrentContext().IsHTML();
    
    if (bIsHTMLMode) {
        // Generate HTML format with proper CSS classes and anchor links
        FString CallTypeClass = TEXT("bp-call-custom-event");
        FString LinkClass = TEXT("custom-event-link");
        
        FString CallKeywordHtml = FString::Printf(TEXT("<span class=\"%s\">Custom Event</span>"), *CallTypeClass);
        FString LinkHtml = FString::Printf(TEXT("<a href=\"#%s\" class=\"graph-link %s\">%s</a>"), 
            *AnchorName, *LinkClass, *FMarkdownSpanSystem::EscapeHtml(FinalLinkTextForDisplay));
        
        LineContent = DisplayTargetPrefix + CallKeywordHtml + TEXT(": ") + LinkHtml + ArgsStr;
        
        if (bWasAlreadyGloballyProcessed) {
            LineContent += TEXT(" <span class=\"repeat-indicator\">(Call site repeat or graph already defined/queued)</span>");
        }
    } else {
        // Generate Markdown format (original logic)
        FString NodeTypeKeywordDisplay = TEXT("Call Custom Event");
        FString BaseLinkText = FString::Printf(TEXT("%s%s: [%s](#%s)"), 
            *DisplayTargetPrefix, *NodeTypeKeywordDisplay, *FinalLinkTextForDisplay, *AnchorName);
        LineContent = BaseLinkText + ArgsStr;
        
        if (bWasAlreadyGloballyProcessed) {
            LineContent += TEXT(" (Call site repeat or graph already defined/queued)");
        }
    }

    OutLines.Add(Self->GenerateMarkdownLine(LineContent, CurrentIndentPrefix));

    if (!bWasAlreadyGloballyProcessed && Self->bCurrentDefineUserGraphsSeparately && Self->CurrentGraphsToDefineSeparatelyPtr && Self->CurrentProcessedSeparateGraphPathsPtr) {
        if (!Self->CurrentProcessedSeparateGraphPathsPtr->Contains(ActualGraphPath)) {
            Self->CurrentGraphsToDefineSeparatelyPtr->AddUnique(MakeTuple(UniqueGraphNameHint, ActualGraphPath, FMarkdownPathTracer::EUserGraphType::CustomEventGraph));
        }
    }
    
    TSharedPtr<FBlueprintPin> OutputExecPin = ExecutableNode->GetExecutionOutputPin();
    if (OutputExecPin && OutputExecPin->LinkedPins.Num() > 0 && OutputExecPin->LinkedPins[0].IsValid()) {
        TSharedPtr<FBlueprintPin> TargetPinOnNext = OutputExecPin->LinkedPins[0];
        const TSharedPtr<FBlueprintNode>* NextNodeInCallerGraphPtr = AllNodes.Find(TargetPinOnNext->NodeGuid);
        if (NextNodeInCallerGraphPtr && NextNodeInCallerGraphPtr->IsValid()) {
            OutNextNodeToTrace = *NextNodeInCallerGraphPtr;
        } else {
            if (!bWasAlreadyGloballyProcessed) {
                 FString EndMessage = FString::Printf(TEXT("[Path ends after call to custom event '%s' - Next node not in selection]"), *FinalLinkTextForDisplay);
                OutLines.Add(Self->GenerateMarkdownLine(EndMessage, Self->CalculateNextPrefix(CurrentIndentPrefix, bIsLastSegment)));
            }
        }
    } else {
        if (!bWasAlreadyGloballyProcessed) {
            FString EndMessage = FString::Printf(TEXT("[Path ends after call to custom event '%s']"), *FinalLinkTextForDisplay);
            OutLines.Add(Self->GenerateMarkdownLine(EndMessage, Self->CalculateNextPrefix(CurrentIndentPrefix, bIsLastSegment)));
        }
    }
    return true;
}

bool FMarkdownPathTracer::ProcessInterfaceCall_Helper(
    FMarkdownPathTracer* Self,
    TSharedPtr<const FBlueprintNode> ExecutableNode, 
    const FString& ActualGraphPath, 
    const FString& UniqueGraphNameHint,
    const FString& DisplayTargetPrefix, 
    const FString& FinalLinkTextForDisplay, 
    const FString& ArgsStr,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, 
    TSet<FString>& InOutProcessedGlobally,
    const FString& CurrentIndentPrefix, 
    bool bIsLastSegment, 
    TArray<FString>& OutLines,
    bool bWasAlreadyGloballyProcessed, 
    TSharedPtr<const FBlueprintNode>& OutNextNodeToTrace)
{
    OutNextNodeToTrace = nullptr;
    FString AnchorName = FMarkdownPathTracer::SanitizeAnchorName(UniqueGraphNameHint);
    
    // Generate interface call format
    FString LineContent;
    
    bool bIsHTMLMode = (Self->ExecPrefix.Contains(TEXT("<")) || Self->ExecPrefix.Contains(TEXT("&"))) || 
                       FMarkdownSpanSystem::GetCurrentContext().IsHTML();
    
    if (bIsHTMLMode) {
        // Generate HTML format for interface calls
        FString CallTypeClass = TEXT("bp-call-interface");
        FString LinkClass = TEXT("interface-link");
        
        FString CallKeywordHtml = FString::Printf(TEXT("<span class=\"%s\">Call Interface</span>"), *CallTypeClass);
        FString LinkHtml = FString::Printf(TEXT("<a href=\"#%s\" class=\"graph-link %s\">%s</a>"), 
            *AnchorName, *LinkClass, *FMarkdownSpanSystem::EscapeHtml(FinalLinkTextForDisplay));
        
        LineContent = DisplayTargetPrefix + CallKeywordHtml + TEXT(": ") + LinkHtml + ArgsStr;
        
        if (bWasAlreadyGloballyProcessed) {
            LineContent += TEXT(" <span class=\"repeat-indicator\">(Interface call repeat or already defined)</span>");
        }
    } else {
        // Generate Markdown format for interface calls
        FString NodeTypeKeywordDisplay = TEXT("Call Interface");
        FString BaseLinkText = FString::Printf(TEXT("%s%s: [%s](#%s)"), 
            *DisplayTargetPrefix, *NodeTypeKeywordDisplay, *FinalLinkTextForDisplay, *AnchorName);
        LineContent = BaseLinkText + ArgsStr;
        
        if (bWasAlreadyGloballyProcessed) {
            LineContent += TEXT(" (Interface call repeat or already defined)");
        }
    }

    OutLines.Add(Self->GenerateMarkdownLine(LineContent, CurrentIndentPrefix));
    
    // Queue interface for definition if needed
    if (!bWasAlreadyGloballyProcessed && Self->bCurrentDefineUserGraphsSeparately && 
        Self->CurrentGraphsToDefineSeparatelyPtr && Self->CurrentProcessedSeparateGraphPathsPtr) {
        if (!Self->CurrentProcessedSeparateGraphPathsPtr->Contains(ActualGraphPath)) {
            UE_LOG(LogPathTracer, Error, TEXT("ProcessInterfaceCall_Helper: QUEUING interface for definition: Hint='%s', Path='%s', Type=Interface"), 
                *UniqueGraphNameHint, *ActualGraphPath);
            Self->CurrentGraphsToDefineSeparatelyPtr->AddUnique(MakeTuple(UniqueGraphNameHint, ActualGraphPath, FMarkdownPathTracer::EUserGraphType::Interface));
        }
    }
    
    // Interface calls end execution path (no following the implementation)
    TSharedPtr<FBlueprintPin> OutputExecPin = ExecutableNode->GetExecutionOutputPin();
    if (OutputExecPin && OutputExecPin->LinkedPins.Num() > 0 && OutputExecPin->LinkedPins[0].IsValid()) {
        TSharedPtr<FBlueprintPin> TargetPinOnNext = OutputExecPin->LinkedPins[0];
        const TSharedPtr<FBlueprintNode>* NextNodeInCallerGraphPtr = AllNodes.Find(TargetPinOnNext->NodeGuid);
        if (NextNodeInCallerGraphPtr && NextNodeInCallerGraphPtr->IsValid()) {
            OutNextNodeToTrace = *NextNodeInCallerGraphPtr;
        } else { 
            if (!bWasAlreadyGloballyProcessed) {
                FString EndMessage = FString::Printf(TEXT("[Path ends after interface call to '%s' - Next node not in selection]"), *FinalLinkTextForDisplay);
                OutLines.Add(Self->GenerateMarkdownLine(EndMessage, Self->CalculateNextPrefix(CurrentIndentPrefix, bIsLastSegment)));
            }
        }
    } else { 
        if (!bWasAlreadyGloballyProcessed) {
            FString EndMessage = FString::Printf(TEXT("[Path ends after interface call to '%s']"), *FinalLinkTextForDisplay);
            OutLines.Add(Self->GenerateMarkdownLine(EndMessage, Self->CalculateNextPrefix(CurrentIndentPrefix, bIsLastSegment)));
        }
    }
    
    return true;
}



bool FMarkdownPathTracer::ProcessCollapsedGraph_Helper(
    FMarkdownPathTracer* Self,
    TSharedPtr<const FBlueprintNode> ExecutableNode, const FString& ActualGraphPath, const FString& UniqueGraphNameHint,
    const FString& DisplayTargetPrefix, const FString& FinalLinkTextForDisplay, const FString& ArgsStr,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, TSet<FString>& InOutProcessedGlobally, TSet<FString>& ProcessedInCurrentPath,
    const FString& CurrentIndentPrefix, bool bIsLastSegment, TArray<FString>& OutLines,
    bool bWasAlreadyGloballyProcessed, TSharedPtr<const FBlueprintNode>& OutNextNodeToTrace,
    const FString& PathTracerCurrentBlueprintContextForArgs) 
{
    OutNextNodeToTrace = nullptr;
    FString NodeTypeKeywordDisplay = TEXT("Collapsed Graph");

    bool bShouldExpandThisNodeInline = Self->bCurrentExpandCompositesInline;
    bool bHandleThisNodeSeparately = Self->bCurrentDefineUserGraphsSeparately && !bShouldExpandThisNodeInline;

    // Declare variables needed for both HTML and Markdown
    FString AnchorName = FMarkdownPathTracer::SanitizeAnchorName(UniqueGraphNameHint);
    FString BaseLinkText = FString::Printf(TEXT("%s%s: [%s](#%s)"), *DisplayTargetPrefix, *NodeTypeKeywordDisplay, *FinalLinkTextForDisplay, *AnchorName);

    if (bHandleThisNodeSeparately) {
        // This logic branch handles graphs that are defined separately (not expanded inline).
        
        FString LineContent;
        
        // NEW: Check the global context to determine which format to generate
        bool bIsHTMLMode = FMarkdownSpanSystem::GetCurrentContext().IsHTML();

        if (bIsHTMLMode)
        {
            // Generate proper HTML with CSS classes and an <a> tag
            FString CallTypeClass = TEXT("bp-call-collapsed-graph");
            FString LinkClass = TEXT("collapsed-graph-link");
            
            FString CallKeywordHtml = FString::Printf(TEXT("<span class=\"%s\">Collapsed Graph</span>"), *CallTypeClass);
            FString LinkHtml = FString::Printf(TEXT("<a href=\"#%s\" class=\"graph-link %s\">%s</a>"), *AnchorName, *LinkClass, *FMarkdownSpanSystem::EscapeHtml(FinalLinkTextForDisplay));
            
            LineContent = DisplayTargetPrefix + CallKeywordHtml + TEXT(": ") + LinkHtml + ArgsStr;
            
            if (bWasAlreadyGloballyProcessed) {
                LineContent += TEXT(" <span class=\"repeat-indicator\">(Call site repeat or graph already defined/queued)</span>");
            }
        }
        else // Fallback to original Markdown generation
        {
            // Reconstruct the original Markdown link for the Markdown context
            FString BaseLinkText_MD = FString::Printf(TEXT("%s%s: [%s](#%s)"), *DisplayTargetPrefix, *NodeTypeKeywordDisplay, *FinalLinkTextForDisplay, *AnchorName);
            LineContent = BaseLinkText_MD + ArgsStr;

            if (bWasAlreadyGloballyProcessed) {
                LineContent += TEXT(" (Call site repeat or graph already defined/queued)");
            }
        }

        // Add the fully constructed line to the output
        OutLines.Add(Self->GenerateMarkdownLine(LineContent, CurrentIndentPrefix));

        if (!bWasAlreadyGloballyProcessed && Self->CurrentGraphsToDefineSeparatelyPtr && Self->CurrentProcessedSeparateGraphPathsPtr) {
            if (!Self->CurrentProcessedSeparateGraphPathsPtr->Contains(ActualGraphPath)) {
                UE_LOG(LogPathTracer, Error, TEXT("ProcessCollapsedGraph_Helper (Separate): QUEUING for definition: Hint='%s', Path='%s', Type=CollapsedGraph"), *UniqueGraphNameHint, *ActualGraphPath);
                Self->CurrentGraphsToDefineSeparatelyPtr->AddUnique(MakeTuple(UniqueGraphNameHint, ActualGraphPath, FMarkdownPathTracer::EUserGraphType::CollapsedGraph));
            }
        }
    }
    
    else if (bShouldExpandThisNodeInline) {
        if (!bWasAlreadyGloballyProcessed) {
            FString InlineHeader = FString::Printf(TEXT("%s[Expanding %s%s: %s]%s"), *Self->ExecPrefix, *DisplayTargetPrefix, *NodeTypeKeywordDisplay, *FinalLinkTextForDisplay, *ArgsStr);
            OutLines.Add(CurrentIndentPrefix + InlineHeader);

            TMap<FString, TSharedPtr<FBlueprintNode>> CompositeNodesMap;
            if (Self->DataExtractorRef.ExtractNodesFromGraph(ActualGraphPath, CompositeNodesMap) && !CompositeNodesMap.IsEmpty()) {
                TSharedPtr<const FBlueprintNode> InternalStartNode = nullptr;
                TSharedPtr<const FBlueprintPin> ActualInputPinForInternalStart = nullptr;
                for (const auto& Pair : CompositeNodesMap) {
                    if (Pair.Value.IsValid() && (Pair.Value->NodeType == TEXT("Tunnel")) && (Pair.Value->Name.Contains(TEXT("Inputs")) || Pair.Value->Name.Contains(TEXT("Input")) )) {
                        TSharedPtr<FBlueprintPin> EntryExecOut = Pair.Value->GetExecutionOutputPin(TEXT(""));
                        if (EntryExecOut.IsValid() && EntryExecOut->LinkedPins.Num() > 0 && EntryExecOut->LinkedPins[0].IsValid()) {
                            TSharedPtr<const FBlueprintPin> FirstPinInsideGraph = EntryExecOut->LinkedPins[0];
                            const TSharedPtr<FBlueprintNode>* StartNodePtr = CompositeNodesMap.Find(FirstPinInsideGraph->NodeGuid);
                            if (StartNodePtr && StartNodePtr->IsValid()) {
                                InternalStartNode = *StartNodePtr;
                                ActualInputPinForInternalStart = FirstPinInsideGraph;
                                break;
                            }
                        }
                    }
                }

                if (InternalStartNode.IsValid()) {
                    FString NextIndentPrefix = CurrentIndentPrefix + (bIsLastSegment ? Self->IndentSpace : Self->LineCont);
                    FString CompositeGraphAssetContext = MarkdownTracerUtils::ExtractSimpleNameFromPath(ActualGraphPath, TEXT(""));
                    if(CompositeGraphAssetContext.Contains(TEXT(":"))) CompositeGraphAssetContext = CompositeGraphAssetContext.Left(CompositeGraphAssetContext.Find(TEXT(":")));
                    if (CompositeGraphAssetContext.EndsWith(TEXT("_C"))) CompositeGraphAssetContext.LeftChopInline(2);

                    TMap<FName, FString> CallSiteArgsForInlineTrace;
                    for (const auto& CompInputPinPair : ExecutableNode->Pins) {
                        const TSharedPtr<FBlueprintPin>& CompInputPin = CompInputPinPair.Value;
                        if (CompInputPin.IsValid() && CompInputPin->IsInput() && CompInputPin->Category != TEXT("exec")) {
                            TSet<FString> TempVisitedForArgResolve;
                            int32 ArgumentResolutionDepth = (CurrentIndentPrefix.Len() / Self->IndentSpace.Len()) + 1;
                            FString ArgValue = Self->DataTracerRef.ResolvePinValueRecursive(
                                CompInputPin, AllNodes, ArgumentResolutionDepth, TempVisitedForArgResolve,
                                ExecutableNode, nullptr, Self->bCurrentTraceDataSymbolically, PathTracerCurrentBlueprintContextForArgs
                            );
                            CallSiteArgsForInlineTrace.Add(FName(*CompInputPin->Name), ArgValue);
                        }
                    }
                    
                    const TMap<FName, FString>* PreviousCallSiteArgs = Self->DataTracerRef.GetCurrentCallsiteArguments();
                    Self->DataTracerRef.SetCurrentCallsiteArguments(&CallSiteArgsForInlineTrace);

                    Self->TracePathRecursive(
                        InternalStartNode, TOptional<FCapturedEventData>(), CompositeNodesMap, InOutProcessedGlobally,
                        ProcessedInCurrentPath, NextIndentPrefix, true, OutLines,
                        CompositeGraphAssetContext,
                        FMarkdownPathTracer::FTraceStepContext(ActualInputPinForInternalStart)
                    );

                    Self->DataTracerRef.SetCurrentCallsiteArguments(PreviousCallSiteArgs);
                } else {
                    OutLines.Add(CurrentIndentPrefix + Self->IndentSpace + Self->ExecPrefix + TEXT("[Warning: Could not find entry point for inline Collapsed Graph expansion]"));
                }
            } else {
                OutLines.Add(CurrentIndentPrefix + Self->IndentSpace + Self->ExecPrefix + TEXT("[Error: Failed to extract nodes for inline Collapsed Graph expansion]"));
            }
        } else {
            FString RepeatMessage = FString::Printf(TEXT("%s%s: %s%s (Previously expanded inline, potential recursion)"),
                *Self->ExecPrefix, *NodeTypeKeywordDisplay, *DisplayTargetPrefix, *FinalLinkTextForDisplay);
            OutLines.Add(CurrentIndentPrefix + RepeatMessage);
        }
    } else {
        FString FallbackMessage = FString::Printf(TEXT("%s%s: %s%s%s"), *Self->ExecPrefix, *NodeTypeKeywordDisplay, *DisplayTargetPrefix, *FinalLinkTextForDisplay, *ArgsStr);
        OutLines.Add(CurrentIndentPrefix + FallbackMessage + TEXT(" (Not expanded, not defined separately)"));
    }

    TSharedPtr<FBlueprintPin> OutputExecPin = ExecutableNode->GetExecutionOutputPin(TEXT(""));
    if (OutputExecPin && OutputExecPin->LinkedPins.Num() > 0 && OutputExecPin->LinkedPins[0].IsValid()) {
        TSharedPtr<FBlueprintPin> TargetPinOnNextNode = OutputExecPin->LinkedPins[0];
        const TSharedPtr<FBlueprintNode>* NextNodeInCallerGraphPtr = AllNodes.Find(TargetPinOnNextNode->NodeGuid);
        if (NextNodeInCallerGraphPtr && NextNodeInCallerGraphPtr->IsValid()) {
            OutNextNodeToTrace = *NextNodeInCallerGraphPtr;
        } else {
            if (!bWasAlreadyGloballyProcessed || bShouldExpandThisNodeInline) {
                FString EndPathPrefix = Self->CalculateNextPrefix(CurrentIndentPrefix, bIsLastSegment);
                if (OutLines.IsEmpty() || !OutLines.Last().Contains(TEXT("[Path ends"))) {
                    OutLines.Add(EndPathPrefix + Self->ExecPrefix + FString::Printf(TEXT("[Path ends after graph '%s' - Next node not found in selection]"),*FinalLinkTextForDisplay));
                }
            }
        }
    } else {
        if (!bWasAlreadyGloballyProcessed || bShouldExpandThisNodeInline) {
            FString EndPathPrefix = Self->CalculateNextPrefix(CurrentIndentPrefix, bIsLastSegment);
            if (OutLines.IsEmpty() || !OutLines.Last().Contains(TEXT("[Path ends"))) {
                OutLines.Add(EndPathPrefix + Self->ExecPrefix + FString::Printf(TEXT("[Path ends after graph '%s']"),*FinalLinkTextForDisplay));
            }
        }
    }
    return true;
}




// --- Class Member Function Definitions ---

FMarkdownPathTracer::FMarkdownPathTracer(FMarkdownDataTracer& InDataTracer, FBlueprintDataExtractor& InDataExtractor)
    : DataTracerRef(InDataTracer)
      , DataExtractorRef(InDataExtractor)
      , ExecPrefix(TEXT("* "))
      , LineCont(TEXT("|   "))
      , BranchJoin(TEXT("|-- "))
      , BranchLast(TEXT("L-- "))
      , IndentSpace(TEXT("    "))
      , MaxTraceDepth(70)
      , CurrentGraphsToDefineSeparatelyPtr(nullptr)
      , CurrentProcessedSeparateGraphPathsPtr(nullptr)
      , PathTracerCurrentBlueprintContext(FString(""))
{
    UE_LOG(LogPathTracer, Log, TEXT("FMarkdownPathTracer: Initialized for pure tracing logic."));
}


// REPLACE the existing TraceExecutionPath method with this cleaned version
TArray<FString> FMarkdownPathTracer::TraceExecutionPath(
    TSharedPtr<const FBlueprintNode> StartNode,
    const TOptional<FCapturedEventData>& CapturedData,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
    TSet<FString>& InOutProcessedGlobally,
    bool bInShouldTraceSymbolicallyForData,
    bool bInDefineUserGraphsSeparately,
    bool bInExpandCompositesInline,
    TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>& OutGraphsToDefineSeparately,
    TSet<FString>& InOutProcessedSeparateGraphPaths,
    const FString& CurrentBlueprintContext
)
{
    TArray<FString> OutputLines;
    TSet<FString> ProcessedInCurrentPath;
    
    if (!StartNode.IsValid())
    {
        OutputLines.Add(GenerateMarkdownLine(TEXT("[Error: Cannot trace from null StartNode]"), TEXT("")));
        return OutputLines;
    }
    if (AllNodes.IsEmpty())
    {
        OutputLines.Add(GenerateMarkdownLine(TEXT("[Error: Node map is empty]"), TEXT("")));
        return OutputLines;
    }
    
    bCurrentTraceDataSymbolically = bInShouldTraceSymbolicallyForData;
    bCurrentDefineUserGraphsSeparately = bInDefineUserGraphsSeparately;
    bCurrentExpandCompositesInline = bInExpandCompositesInline;
    CurrentGraphsToDefineSeparatelyPtr = &OutGraphsToDefineSeparately;
    CurrentProcessedSeparateGraphPathsPtr = &InOutProcessedSeparateGraphPaths;
    PathTracerCurrentBlueprintContext = CurrentBlueprintContext;

    UE_LOG(LogPathTracer, Log, TEXT("Starting Execution Trace from Node: %s (%s) in Context: '%s'. DataSymbolic=%d, DefineSep=%d, ExpandInline=%d"),
        StartNode->Name.IsEmpty() ? *StartNode->NodeType : *StartNode->Name, *StartNode->Guid.Left(8),
        *PathTracerCurrentBlueprintContext,
        bCurrentTraceDataSymbolically, bCurrentDefineUserGraphsSeparately, bCurrentExpandCompositesInline);

    FTraceStepContext InitialStepContext;
    
    TracePathRecursive(
        StartNode,
        CapturedData,
        AllNodes,
        InOutProcessedGlobally,
        ProcessedInCurrentPath,
        TEXT(""),
        true,
        OutputLines,
        PathTracerCurrentBlueprintContext,
        InitialStepContext 
    );

    CurrentGraphsToDefineSeparatelyPtr = nullptr; 
    CurrentProcessedSeparateGraphPathsPtr = nullptr;

    return OutputLines;
}
FMarkdownPathTracer::EUserGraphType FMarkdownPathTracer::IsInternalUserGraph(
    TSharedPtr<const FBlueprintNode> Node,
    FString& OutGraphPath,
    FString& OutGraphNameHint
) const
{
    if (!Node.IsValid())
    {
        UE_LOG(LogPathTracer, Warning, TEXT("IsInternalUserGraph: Received an invalid Node shared pointer."));
        return EUserGraphType::Unknown;
    }

    OutGraphPath = TEXT("");
    OutGraphNameHint = TEXT("");
    EUserGraphType DetectedType = EUserGraphType::Unknown;

    UEdGraphNode* EdGraphNode = Node->GetEdGraphNode();
    if (!EdGraphNode)
    {
        UE_LOG(LogPathTracer, Warning, TEXT("IsInternalUserGraph: Node '%s' (GUID:%s, Type:%s) has no valid OriginalEdGraphNode. Cannot determine graph type."),
            *Node->Name, *Node->Guid.Left(8), *Node->NodeType);
        return EUserGraphType::Unknown;
    }

    UBlueprint* CallingNodeOwningBlueprint = FBlueprintEditorUtils::FindBlueprintForNode(EdGraphNode);
    FString CallingBlueprintName = CallingNodeOwningBlueprint ? CallingNodeOwningBlueprint->GetName() : TEXT("UnknownCallingBP");
    CallingBlueprintName.RemoveFromStart(TEXT("Default__"));
    if (CallingBlueprintName.EndsWith(TEXT("_C"))) CallingBlueprintName.LeftChopInline(2);

    UE_LOG(LogPathTracer, Log, TEXT("IsInternalUserGraph: Processing Node '%s' (Title: '%s', OriginalType: %s, CallerBP: %s) for user graph identification."),
        *Node->Guid.Left(8), *Node->Name, *Node->NodeType, *CallingBlueprintName);

    if (Node->NodeType == TEXT("MacroInstance"))
    {
        DetectedType = CheckMacroInstanceType_Helper(Node, CallingBlueprintName, OutGraphPath, OutGraphNameHint);
    }
    else if (Node->NodeType == TEXT("Composite"))
    {
        DetectedType = CheckCompositeNodeType_Helper(Node, CallingBlueprintName, OutGraphPath, OutGraphNameHint);
    }
    else if (Node->NodeType == TEXT("CallFunction") || Node->NodeType == TEXT("CallParentFunction"))
    {
        DetectedType = CheckCallFunctionType_Helper(Node, CallingNodeOwningBlueprint, CallingBlueprintName, OutGraphPath, OutGraphNameHint);
    }

    UE_LOG(LogPathTracer, Log, TEXT("IsInternalUserGraph FINAL RESULT for Node '%s' (Title: '%s'): DetectedType=%d, OutGraphPath='%s', OutGraphNameHint='%s'"),
        *Node->Guid.Left(8), *Node->Name, static_cast<int>(DetectedType), *OutGraphPath, *OutGraphNameHint);
    return DetectedType;
}

bool FMarkdownPathTracer::HandleUserGraphNode(
    TSharedPtr<const FBlueprintNode> ExecutableNode,
    const FString& ExecutableGuid,
    EUserGraphType InternalGraphNodeType,
    const FString& ActualGraphPath,
    const FString& UniqueGraphNameHint,
    bool bIsInternalGraphCall,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
    TSet<FString>& InOutProcessedGlobally,
    TSet<FString>& ProcessedInCurrentPath,
    const FString& CurrentIndentPrefixMarkdown,
    bool bIsLastSegmentMarkdown,
    TArray<FString>& OutLines,
    bool bWasAlreadyGloballyProcessed,
    TSharedPtr<const FBlueprintNode>& OutNextNodeToTrace
) {
    OutNextNodeToTrace = nullptr;

    if (!bIsInternalGraphCall || InternalGraphNodeType == EUserGraphType::Unknown) {
        return false; 
    }

    // CRITICAL FIX: Add discovered user graphs to collection arrays during tracing
    if (bCurrentDefineUserGraphsSeparately && CurrentGraphsToDefineSeparatelyPtr && CurrentProcessedSeparateGraphPathsPtr) {
        if (!CurrentProcessedSeparateGraphPathsPtr->Contains(ActualGraphPath)) {
            
            // Check if this is a pure macro/function that should be collected
            bool bShouldCollectThisGraph = false;
            
            if (InternalGraphNodeType == EUserGraphType::Macro) {
                // For macros, check if the node itself is pure
                bShouldCollectThisGraph = ExecutableNode->IsPure();
                UE_LOG(LogPathTracer, Warning, TEXT("DISCOVERED Pure Macro during tracing: %s (Pure: %d)"), 
                    *UniqueGraphNameHint, bShouldCollectThisGraph);
            }
            else if (InternalGraphNodeType == EUserGraphType::Function) {
                // For functions, check the bIsPureFunc property
                const FString* PurityProp = ExecutableNode->RawProperties.Find(TEXT("bIsPureFunc"));
                bShouldCollectThisGraph = PurityProp && PurityProp->ToBool();
                UE_LOG(LogPathTracer, Warning, TEXT("DISCOVERED Function during tracing: %s (Pure: %d)"), 
                    *UniqueGraphNameHint, bShouldCollectThisGraph);
            }
            else {
                // For other types (Custom Events, Collapsed Graphs), always collect
                bShouldCollectThisGraph = true;
                UE_LOG(LogPathTracer, Warning, TEXT("DISCOVERED User Graph during tracing: %s (Type: %d)"), 
                    *UniqueGraphNameHint, static_cast<int32>(InternalGraphNodeType));
            }
            
            if (bShouldCollectThisGraph) {
                CurrentGraphsToDefineSeparatelyPtr->AddUnique(MakeTuple(UniqueGraphNameHint, ActualGraphPath, InternalGraphNodeType));
                UE_LOG(LogPathTracer, Error, TEXT("QUEUED for definition during tracing: Hint='%s', Path='%s', Type=%d"), 
                    *UniqueGraphNameHint, *ActualGraphPath, static_cast<int32>(InternalGraphNodeType));
            }
        }
    }

    // Rest of the method remains the same as before...
    FString TargetDisplayStringForPrefix = TEXT("");
    FString ItemNamePart = UniqueGraphNameHint;
    FString DefiningBlueprintNamePart = TEXT("");

    int32 SeparatorPos = -1;
    UniqueGraphNameHint.FindLastChar(TEXT('.'), SeparatorPos);
    if (SeparatorPos == INDEX_NONE) {
        UniqueGraphNameHint.FindLastChar(TEXT(':'), SeparatorPos);
    }
    if (SeparatorPos != INDEX_NONE) {
        DefiningBlueprintNamePart = UniqueGraphNameHint.Left(SeparatorPos);
        ItemNamePart = UniqueGraphNameHint.Mid(SeparatorPos + 1);
    }

    if (InternalGraphNodeType == EUserGraphType::CollapsedGraph) {
        ItemNamePart = ExecutableNode->Name.Replace(TEXT("\n"), TEXT(" "));
        if (ItemNamePart.IsEmpty() || ItemNamePart == ExecutableNode->NodeType.Replace(TEXT("\n"), TEXT(" "))) {
            ItemNamePart = UniqueGraphNameHint.Mid(SeparatorPos != INDEX_NONE ? SeparatorPos + 1 : 0);
            ItemNamePart.ReplaceInline(TEXT("\n"), TEXT(" "), ESearchCase::CaseSensitive);
            if (ItemNamePart.IsEmpty()) ItemNamePart = FPaths::GetCleanFilename(ActualGraphPath);
            if (ItemNamePart.EndsWith(TEXT("_Graph"))) ItemNamePart.LeftChopInline(6);
        }
    } else {
        ItemNamePart.ReplaceInline(TEXT("\n"), TEXT(" "), ESearchCase::CaseSensitive);
    }

    TSharedPtr<const FBlueprintPin> TargetPin = ExecutableNode->GetPin(TEXT("self"));
    if (!TargetPin.IsValid() && ExecutableNode->NodeType != TEXT("CallParentFunction")) {
        TargetPin = ExecutableNode->GetPin(TEXT("Target"));
    }

    if (TargetPin.IsValid()) {
        TSet<FString> VisitedPinsForTargetTrace;
        TargetDisplayStringForPrefix = DataTracerRef.TraceTargetPin(
            TargetPin, AllNodes, CurrentIndentPrefixMarkdown.Len() / IndentSpace.Len() + 1, 
            VisitedPinsForTargetTrace, PathTracerCurrentBlueprintContext); 
    }

    bool bIsSelfTarget = TargetDisplayStringForPrefix.IsEmpty() || TargetDisplayStringForPrefix == FMarkdownSpan::Variable(TEXT("`self`"));
    bool bIsDifferentBlueprint = !DefiningBlueprintNamePart.IsEmpty() && DefiningBlueprintNamePart != PathTracerCurrentBlueprintContext;
    FString FinalLinkTextForDisplay;

    if (!bIsSelfTarget || (bIsDifferentBlueprint && ExecutableNode->NodeType != TEXT("CallParentFunction"))) {
        FinalLinkTextForDisplay = UniqueGraphNameHint.Replace(TEXT(":"), TEXT("."));
    } else {
        FinalLinkTextForDisplay = ItemNamePart;
    }
    if(FinalLinkTextForDisplay.IsEmpty()) FinalLinkTextForDisplay = TEXT("UnknownGraphItem");

    FString DisplayTargetPrefix = TEXT("");
    if (!bIsSelfTarget) {
        DisplayTargetPrefix = TargetDisplayStringForPrefix + TEXT(".");
    }
    if (ExecutableNode->NodeType == TEXT("CallParentFunction")) {
        DisplayTargetPrefix = TEXT("");
    }
    
    FString ArgsStr = TEXT("()");
    if (InternalGraphNodeType != EUserGraphType::Unknown)
    {
        TSet<FString> VisitedPinsForLinkArgs;
        TSet<FName> ExclusionsForArgs;
        if (TargetPin.IsValid()) ExclusionsForArgs.Add(TargetPin->PinName);

        FString RawArgsStr = MarkdownFormattingUtils::FormatArgumentsForTrace(
            ExecutableNode, &DataTracerRef, AllNodes,
            CurrentIndentPrefixMarkdown.Len() / IndentSpace.Len() + 1, 
            VisitedPinsForLinkArgs, ExclusionsForArgs, nullptr, nullptr,
            bCurrentTraceDataSymbolically, PathTracerCurrentBlueprintContext);
        if (!RawArgsStr.IsEmpty()) { ArgsStr = FString::Printf(TEXT("(%s)"), *RawArgsStr); }
    }
    
    switch (InternalGraphNodeType)
    {
        case EUserGraphType::Function:
            return ProcessUserFunctionCall_Helper(this, ExecutableNode, ActualGraphPath, UniqueGraphNameHint, DisplayTargetPrefix, FinalLinkTextForDisplay, ArgsStr, AllNodes, InOutProcessedGlobally, CurrentIndentPrefixMarkdown, bIsLastSegmentMarkdown, OutLines, bWasAlreadyGloballyProcessed, OutNextNodeToTrace);
        case EUserGraphType::Macro:
            return ProcessMacroCall_Helper(this, ExecutableNode, ActualGraphPath, UniqueGraphNameHint, DisplayTargetPrefix, FinalLinkTextForDisplay, ArgsStr, AllNodes, InOutProcessedGlobally, CurrentIndentPrefixMarkdown, bIsLastSegmentMarkdown, OutLines, bWasAlreadyGloballyProcessed, OutNextNodeToTrace);
        case EUserGraphType::CustomEventGraph:
            return ProcessCustomEventCall_Helper(this, ExecutableNode, ActualGraphPath, UniqueGraphNameHint, DisplayTargetPrefix, FinalLinkTextForDisplay, ArgsStr, AllNodes, InOutProcessedGlobally, CurrentIndentPrefixMarkdown, bIsLastSegmentMarkdown, OutLines, bWasAlreadyGloballyProcessed, OutNextNodeToTrace);
        case EUserGraphType::CollapsedGraph:
            return ProcessCollapsedGraph_Helper(this, ExecutableNode, ActualGraphPath, UniqueGraphNameHint, DisplayTargetPrefix, FinalLinkTextForDisplay, ArgsStr, AllNodes, InOutProcessedGlobally, ProcessedInCurrentPath, CurrentIndentPrefixMarkdown, bIsLastSegmentMarkdown, OutLines, bWasAlreadyGloballyProcessed, OutNextNodeToTrace, PathTracerCurrentBlueprintContext);
    case EUserGraphType::Interface:
        return ProcessInterfaceCall_Helper(this, ExecutableNode, ActualGraphPath, UniqueGraphNameHint, DisplayTargetPrefix, FinalLinkTextForDisplay, ArgsStr, AllNodes, InOutProcessedGlobally, CurrentIndentPrefixMarkdown, bIsLastSegmentMarkdown, OutLines, bWasAlreadyGloballyProcessed, OutNextNodeToTrace);
   
        
        default: // EUserGraphType::Unknown
             UE_LOG(LogPathTracer, Warning, TEXT("HandleUserGraphNode: Called with EUserGraphType::Unknown for Node %s. This should not happen if bIsInternalGraphCall was true."), *ExecutableNode->Name);
            return false;
    }
}



void FMarkdownPathTracer::TracePathRecursive(
    TSharedPtr<const FBlueprintNode> CurrentNode,
    const TOptional<FCapturedEventData>& CurrentCapturedData,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
    TSet<FString>& InOutProcessedGlobally,
    TSet<FString>& ProcessedInCurrentPath,
    const FString& CurrentIndentPrefix,
    bool bIsLastSegment,
    TArray<FString>& OutLines,
    const FString& CurrentBlueprintContext,
    const FTraceStepContext& StepContext        
)
{
    const TCHAR* CurrentNodeNameForLog = CurrentNode.IsValid() ? *(CurrentNode->Name) : TEXT("NULL_NODE");
    const TCHAR* CurrentNodeGuidForLog = CurrentNode.IsValid() ? *(CurrentNode->Guid.Left(8)) : TEXT("NULL_GUID");
    int32 CurrentDepthValue = CurrentIndentPrefix.Len() / IndentSpace.Len();

    UE_LOG(LogPathTracer, Verbose, TEXT("%sTracePathRecursive ENTRY: Node='%s'(%s), Depth:%d, Markdown-only"),
        *CurrentIndentPrefix, CurrentNodeNameForLog, CurrentNodeGuidForLog, CurrentDepthValue);

    if (CurrentDepthValue > MaxTraceDepth) {
        FString Message = FString::Printf(TEXT("[Trace Depth Limit Reached (%d)]"), MaxTraceDepth);
        OutLines.Add(GenerateMarkdownLine(Message, CurrentIndentPrefix));
        return;
    }

    TSharedPtr<const FBlueprintNode> ExecutableNode = nullptr;
    TSharedPtr<const FBlueprintPin> TargetPinForExecutable = nullptr;
    
    // Find ExecutableNode logic (unchanged from original)
    if (StepContext.IntendedEntryPin.IsValid() && CurrentNode.IsValid() && StepContext.IntendedEntryPin->NodeGuid == CurrentNode->Guid)
    {
         if (CurrentNode->NodeType != TEXT("Knot") && CurrentNode->NodeType != TEXT("Comment") && !CurrentNode->IsPure()) {
             ExecutableNode = CurrentNode; TargetPinForExecutable = StepContext.IntendedEntryPin;
         } else {
             Tie(ExecutableNode, TargetPinForExecutable) = _find_next_executable_node(CurrentNode, AllNodes, OutLines, CurrentIndentPrefix);
             if (ExecutableNode.IsValid() && StepContext.IntendedEntryPin.IsValid() && ExecutableNode->Guid == StepContext.IntendedEntryPin->NodeGuid) {
                 TargetPinForExecutable = StepContext.IntendedEntryPin;
             }
         }
    } else {
        if (!CurrentNode.IsValid()) { return; }
        Tie(ExecutableNode, TargetPinForExecutable) = _find_next_executable_node(CurrentNode, AllNodes, OutLines, CurrentIndentPrefix);
    }

    if (!ExecutableNode.IsValid()) {
        bool bMessageAlreadyAdded = !OutLines.IsEmpty() && (OutLines.Last().Contains(TEXT("[Path ended")) || OutLines.Last().Contains(TEXT("[Skipped nodes")));
        if (!bMessageAlreadyAdded) {
            FString Message = TEXT("[Path ended - no further executable node found]");
            OutLines.Add(GenerateMarkdownLine(Message, CurrentIndentPrefix));
        }
        return;
    }
    
    const FString ExecutableGuid = ExecutableNode->Guid;

    bool bWasAlreadyGloballyProcessed = false;
    bool bAddedToCurrentPathThisCall = false; 
    if (HandleGloballyProcessedNode(ExecutableNode, TargetPinForExecutable, ExecutableGuid, InOutProcessedGlobally, ProcessedInCurrentPath, CurrentIndentPrefix, OutLines, bWasAlreadyGloballyProcessed, CurrentNode.IsValid() ? CurrentNode->Guid : FString(), bAddedToCurrentPathThisCall, AllNodes)) {  // ADD , AllNodes
        return; 
    }
    if (!ProcessedInCurrentPath.Contains(ExecutableGuid)) { ProcessedInCurrentPath.Add(ExecutableGuid); bAddedToCurrentPathThisCall = true; }
    InOutProcessedGlobally.Add(ExecutableGuid);

    // REMOVED: All HTML prefix type calculation - Markdown-only now

    FString TargetGraphPath = TEXT(""), TargetGraphNameHint = TEXT("");
    EUserGraphType TargetGraphType = IsInternalUserGraph(ExecutableNode, TargetGraphPath, TargetGraphNameHint);
    bool bIsInternalGraphCall = (TargetGraphType != FMarkdownPathTracer::EUserGraphType::Unknown);

    // Only add generic formatted description if NOT handled by user graph logic
    if (!bIsInternalGraphCall) {
        TSet<FString> VisitedDataPinsForNode; 
        const TOptional<FCapturedEventData>& DataToPass = (CurrentCapturedData.IsSet() && CurrentCapturedData->BoundEventOwnerClassPath.Contains(ExecutableGuid)) ? CurrentCapturedData : TOptional<FCapturedEventData>();
        FString FormattedDesc = FMarkdownNodeFormatter::FormatNodeDescription(ExecutableNode, DataToPass, DataTracerRef, AllNodes, VisitedDataPinsForNode, false, bCurrentTraceDataSymbolically, CurrentBlueprintContext);
    
        UE_LOG(LogPathTracer, Log, TEXT("TracePathRecursive: Adding generic FormattedDesc for Node: %s (%s), Content: [%s]"), 
               *(ExecutableNode->Name), *(ExecutableNode->Guid.Left(8)), *FormattedDesc);
    
        if (!FormattedDesc.IsEmpty()) {
            OutLines.Add(GenerateMarkdownLine(FormattedDesc, CurrentIndentPrefix));
        }
        else if (ExecutableNode.IsValid() && !ExecutableNode->IsPure() && ExecutableNode->NodeType != TEXT("Knot") && ExecutableNode->NodeType != TEXT("Comment")) {
            UE_LOG(LogPathTracer, Warning, TEXT("%sExecutable Node %s (%s) formatted as empty."), *CurrentIndentPrefix, *ExecutableGuid, *(ExecutableNode->NodeType));
        }
    }

    TSharedPtr<const FBlueprintNode> NextNodeToTraceAfterSpecialHandling = nullptr;
    
    bool bNodeHandledByGraphLogic = HandleUserGraphNode(ExecutableNode, ExecutableGuid, TargetGraphType, TargetGraphPath, TargetGraphNameHint, bIsInternalGraphCall, AllNodes, InOutProcessedGlobally, ProcessedInCurrentPath, CurrentIndentPrefix, bIsLastSegment, OutLines, bWasAlreadyGloballyProcessed, NextNodeToTraceAfterSpecialHandling);
    
    if (bNodeHandledByGraphLogic) {
        if (NextNodeToTraceAfterSpecialHandling.IsValid()) {
            TracePathRecursive( 
                NextNodeToTraceAfterSpecialHandling, 
                TOptional<FCapturedEventData>(), 
                AllNodes, 
                InOutProcessedGlobally, 
                ProcessedInCurrentPath, 
                CalculateNextPrefix(CurrentIndentPrefix, bIsLastSegment),
                true, // A linear continuation from a call is effectively the "last" of that call's line
                OutLines, 
                CurrentBlueprintContext, 
                FTraceStepContext(NextNodeToTraceAfterSpecialHandling->GetExecutionInputPin())
            );
        }
    } else {
        ProcessNodeFormattingAndBranching(ExecutableNode, ExecutableGuid, CurrentCapturedData, AllNodes, InOutProcessedGlobally, ProcessedInCurrentPath, CurrentIndentPrefix, bIsLastSegment, OutLines, CurrentBlueprintContext, StepContext);
    }

    if (bAddedToCurrentPathThisCall) { ProcessedInCurrentPath.Remove(ExecutableGuid); }
    UE_LOG(LogPathTracer, Verbose, TEXT("%sTracePathRecursive EXIT for Node %s"), *CurrentIndentPrefix, *ExecutableGuid);
}



TTuple<TSharedPtr<const FBlueprintNode>, TSharedPtr<const FBlueprintPin>> FMarkdownPathTracer::_find_next_executable_node(
	TSharedPtr<const FBlueprintNode> FromNode,
	const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
	TArray<FString>& OutLines,
	const FString& SearchIndentPrefix
)
{
    if (!FromNode.IsValid())
    {
        UE_LOG(LogPathTracer, Log, TEXT("%s>>> _find_next ENTER: Started with NULL node."), *SearchIndentPrefix);
        return TTuple<TSharedPtr<const FBlueprintNode>, TSharedPtr<const FBlueprintPin>>();
    }

    UE_LOG(LogPathTracer, Log, TEXT("%s>>> _find_next ENTER: Searching from %s (%s)"), *SearchIndentPrefix, *(FromNode->Guid.Left(8)), *(FromNode->NodeType));

    const bool bFromNodeIsKnot = (FromNode->NodeType == TEXT("Knot"));
    const bool bFromNodeIsComment = (FromNode->NodeType == TEXT("Comment"));
    const bool bFromNodeIsPure = FromNode->IsPure();

    if (!bFromNodeIsKnot && !bFromNodeIsComment && !bFromNodeIsPure)
    {
        UE_LOG(LogPathTracer, Log, TEXT("%s<<< _find_next EXIT: Input node %s (%s) is already executable."), *SearchIndentPrefix, *(FromNode->Guid.Left(8)), *(FromNode->NodeType));
        return MakeTuple(FromNode, TSharedPtr<const FBlueprintPin>(nullptr));
    }
    if (bFromNodeIsPure || bFromNodeIsComment)
    {
        UE_LOG(LogPathTracer, Log, TEXT("%s<<< _find_next EXIT: Input node %s (%s) is Pure or Comment, cannot skip execution flow."), *SearchIndentPrefix, *(FromNode->Guid.Left(8)), *(FromNode->NodeType));
        return TTuple<TSharedPtr<const FBlueprintNode>, TSharedPtr<const FBlueprintPin>>();
    }

    const int MaxSearchDepthInternal = 20;
    int CurrentSearchDepth = 0;
    TSharedPtr<const FBlueprintNode> CurrentSearchNode = FromNode;
    TSet<FString> VisitedInSearch;

    VisitedInSearch.Add(CurrentSearchNode->Guid);
    UE_LOG(LogPathTracer, Verbose, TEXT("%s  _find_next: Added initial Knot node %s to VisitedInSearch."), *SearchIndentPrefix, *(CurrentSearchNode->Guid.Left(8)));

    while (CurrentSearchNode.IsValid() && CurrentSearchDepth < MaxSearchDepthInternal)
    {
        const FString SearchNodeGuid = CurrentSearchNode->Guid;
        UE_LOG(LogPathTracer, Verbose, TEXT("%s  _find_next Loop Iteration %d: CurrentSearchNode=%s (Knot)"), *SearchIndentPrefix, CurrentSearchDepth, *(SearchNodeGuid.Left(8)));

        TSharedPtr<const FBlueprintPin> ExecPinToFollow = CurrentSearchNode->GetExecutionOutputPin(TEXT(""));

        if (!ExecPinToFollow || ExecPinToFollow->LinkedPins.Num() == 0 || !ExecPinToFollow->LinkedPins[0].IsValid())
        {
            UE_LOG(LogPathTracer, Verbose, TEXT("%s_find_next: Knot %s has no valid outgoing execution link. Path ends while skipping."), *SearchIndentPrefix, *(SearchNodeGuid.Left(8)));
            if (CurrentSearchDepth > 0) {
                OutLines.Add(GenerateMarkdownLine(TEXT("[Skipped nodes ended in a dead end]"), SearchIndentPrefix));
            }
            return TTuple<TSharedPtr<const FBlueprintNode>, TSharedPtr<const FBlueprintPin>>();
        }

        TSharedPtr<const FBlueprintPin> TargetPinOnNextNode = ExecPinToFollow->LinkedPins[0];
        const FString NextNodeGuid = TargetPinOnNextNode->NodeGuid;

        const TSharedPtr<FBlueprintNode>* NextNodeInChainPtr = AllNodes.Find(NextNodeGuid);

        if (!NextNodeInChainPtr || !NextNodeInChainPtr->IsValid())
        {
             UE_LOG(LogPathTracer, Verbose, TEXT("%s_find_next: Next node %s is outside the current map. Path ends while skipping."), *SearchIndentPrefix, *(NextNodeGuid.Left(8)));
             if (CurrentSearchDepth > 0) {
                OutLines.Add(GenerateMarkdownLine(FString::Printf(TEXT("[Skipped nodes led outside selection -> %s]"), *(NextNodeGuid.Left(8))), SearchIndentPrefix));
             }
            return TTuple<TSharedPtr<const FBlueprintNode>, TSharedPtr<const FBlueprintPin>>();
        }

        TSharedPtr<const FBlueprintNode> NextNodeInChain = *NextNodeInChainPtr;

        if (VisitedInSearch.Contains(NextNodeGuid))
        {
             UE_LOG(LogPathTracer, Warning, TEXT("%s  _find_next: Loop detected during skip search. Next node %s (%s) is already in VisitedInSearch. Stopping."), *SearchIndentPrefix, *(NextNodeInChain->NodeType), *(NextNodeGuid.Left(8)));
             OutLines.Add(GenerateMarkdownLine(FString::Printf(TEXT("[Execution loop during skip to `%s` (%s)]"), *(NextNodeInChain->Name), *(NextNodeGuid.Left(8))), SearchIndentPrefix));
             return TTuple<TSharedPtr<const FBlueprintNode>, TSharedPtr<const FBlueprintPin>>();
        }
        VisitedInSearch.Add(NextNodeGuid);
        UE_LOG(LogPathTracer, Verbose, TEXT("%s  _find_next: Moved to node %s. Added to VisitedInSearch."), *SearchIndentPrefix, *(NextNodeGuid.Left(8)));
        CurrentSearchDepth++;

        const bool bTargetIsKnot = (NextNodeInChain->NodeType == TEXT("Knot"));
        const bool bTargetIsComment = (NextNodeInChain->NodeType == TEXT("Comment"));
        const bool bTargetIsPure = NextNodeInChain->IsPure();

        if (!bTargetIsKnot && !bTargetIsComment && !bTargetIsPure)
        {
            UE_LOG(LogPathTracer, Log, TEXT("%s<<< _find_next EXIT: Found executable node %s (%s) via TargetPin '%s' after skipping %d nodes."),
                *SearchIndentPrefix,
                *(NextNodeInChain->NodeType),
                *(NextNodeGuid.Left(8)),
                TargetPinOnNextNode.IsValid() ? *(TargetPinOnNextNode->Name) : TEXT("UNKNOWN"),
                CurrentSearchDepth);
            return MakeTuple(NextNodeInChain, TargetPinOnNextNode);
        }
        if (bTargetIsPure || bTargetIsComment)
        {
            UE_LOG(LogPathTracer, Log, TEXT("%s<<< _find_next EXIT: Path ended at Pure/Comment node %s (%s) while skipping."), *SearchIndentPrefix, *(NextNodeGuid.Left(8)), *(NextNodeInChain->NodeType));
            if (CurrentSearchDepth > 0) {
                OutLines.Add(GenerateMarkdownLine(FString::Printf(TEXT("[Skipped nodes ended on non-executable node `%s`]"), *(NextNodeInChain->Name)), SearchIndentPrefix));
            }
            return TTuple<TSharedPtr<const FBlueprintNode>, TSharedPtr<const FBlueprintPin>>();
        }
		CurrentSearchNode = NextNodeInChain;
    }

    if (CurrentSearchDepth >= MaxSearchDepthInternal)
    {
        UE_LOG(LogPathTracer, Warning, TEXT("%s_find_next: Max search depth (%d) reached while skipping nodes from %s. Stopping search."), *SearchIndentPrefix, MaxSearchDepthInternal, FromNode.IsValid() ? *(FromNode->Guid.Left(8)) : TEXT("NULL"));
        OutLines.Add(GenerateMarkdownLine(FString::Printf(TEXT("[Trace Depth Limit Reached (%d) while skipping nodes]"), MaxSearchDepthInternal), SearchIndentPrefix));
    }

    UE_LOG(LogPathTracer, Log, TEXT("%s<<< _find_next EXIT: No executable node found within depth limit or path ended."), *SearchIndentPrefix);
    return TTuple<TSharedPtr<const FBlueprintNode>, TSharedPtr<const FBlueprintPin>>();
}

FString FMarkdownPathTracer::CalculateNextPrefix(
	const FString& CurrentPrefix,
	bool bIsLastSegment
) const
{
	return CurrentPrefix + (bIsLastSegment ? IndentSpace : LineCont);
}

TTuple<TSharedPtr<const FBlueprintNode>, TSharedPtr<const FBlueprintPin>> FMarkdownPathTracer::HandleInitialChecksAndFindExecutable(
    TSharedPtr<const FBlueprintNode> CurrentNode,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
    int32 CurrentDepth,
    const FString& CurrentIndentPrefix,
    TArray<FString>& OutLines
) {
    if (!CurrentNode) {
        UE_LOG(LogPathTracer, Warning, TEXT("%sTracePathRecursive: Path ended: Reached null node."), *CurrentIndentPrefix);
        return TTuple<TSharedPtr<const FBlueprintNode>, TSharedPtr<const FBlueprintPin>>();
    }
    
    if (CurrentDepth > MaxTraceDepth) {
        // CLEANED: Only Markdown output
        OutLines.Add(GenerateMarkdownLine(FString::Printf(TEXT("[Trace Depth Limit Reached (%d)]"), MaxTraceDepth), CurrentIndentPrefix));
        UE_LOG(LogPathTracer, Warning, TEXT("%sTracePathRecursive: Path ended: Max depth %d reached at Node %s."), *CurrentIndentPrefix, MaxTraceDepth, *(CurrentNode->Guid));
        return TTuple<TSharedPtr<const FBlueprintNode>, TSharedPtr<const FBlueprintPin>>();
    }
    
    return _find_next_executable_node(CurrentNode, AllNodes, OutLines, CurrentIndentPrefix);
}

bool FMarkdownPathTracer::HandlePathLoop(
    TSharedPtr<const FBlueprintNode> NodeToCheck,
    TSet<FString>& ProcessedInCurrentPath,
    const FString& CurrentIndentPrefix,
    TArray<FString>& OutLines
) {
    if (!NodeToCheck.IsValid()) return false;

    const FString NodeGuid = NodeToCheck->Guid;
    if (ProcessedInCurrentPath.Contains(NodeGuid)) {
        FString NodeName = NodeToCheck->Name.IsEmpty() ? NodeToCheck->NodeType : NodeToCheck->Name;
        
        // CLEANED: Only Markdown output - this is critical loop detection!
        OutLines.Add(GenerateMarkdownLine(FString::Printf(TEXT("[Execution loop back to: `%s` (%s)]"), 
            *NodeName, *(NodeGuid.Left(8))), CurrentIndentPrefix));
        
        UE_LOG(LogPathTracer, Warning, TEXT("%sTracePathRecursive: EXIT - Path loop detected back to Node %s."), 
            *CurrentIndentPrefix, *NodeGuid);
        return true;
    }
    
    ProcessedInCurrentPath.Add(NodeGuid);
    UE_LOG(LogPathTracer, Verbose, TEXT("%sAdded %s to ProcessedInCurrentPath (within HandlePathLoop)."), 
        *CurrentIndentPrefix, *NodeGuid);
    return false;
}
// REPLACE the existing HandleGloballyProcessedNode with this cleaned version
bool FMarkdownPathTracer::HandleGloballyProcessedNode(
    TSharedPtr<const FBlueprintNode> ExecutableNode,
    TSharedPtr<const FBlueprintPin> TargetPinForExecutable,
    const FString& ExecutableGuid,
    TSet<FString>& InOutProcessedGlobally,
    TSet<FString>& ProcessedInCurrentPath,
    const FString& CurrentIndentPrefix,
    TArray<FString>& OutLines,
    bool& bWasAlreadyGloballyProcessed,
    const FString& CurrentGuid,
    bool bAddedExecutableToPath,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes 
) {
    bWasAlreadyGloballyProcessed = InOutProcessedGlobally.Contains(ExecutableGuid);
    if (bWasAlreadyGloballyProcessed) {
        FString NodeName = ExecutableNode->Name.IsEmpty() ? ExecutableNode->NodeType : ExecutableNode->Name;
        FString NodeGuidShort = ExecutableGuid.Left(8);
        FString Message = "";

        FString SimpleMacroNameForCheck;
        if (ExecutableNode->NodeType == TEXT("MacroInstance")) {
            const FString* MacroPathPtr = ExecutableNode->RawProperties.Find(TEXT("MacroGraphReference"));
            if (MacroPathPtr && !MacroPathPtr->IsEmpty()) {
                SimpleMacroNameForCheck = MarkdownTracerUtils::ExtractSimpleNameFromPath(*MacroPathPtr, PathTracerCurrentBlueprintContext);
            }
            if (SimpleMacroNameForCheck.IsEmpty() || !SimpleMacroNameForCheck.Contains(TEXT("StandardMacros"))){
                 FString TempNodeName = NodeName;
                 TempNodeName.RemoveSpacesInline(); 
                 if(!TempNodeName.IsEmpty()) SimpleMacroNameForCheck = TempNodeName;
            }
        } else if (ExecutableNode->NodeType == TEXT("K2Node_MultiGate")) { 
            SimpleMacroNameForCheck = TEXT("MultiGate");
        }
        
        FString ActualTargetPinName = TEXT("DefaultExecute");
        if (TargetPinForExecutable.IsValid()) {
            ActualTargetPinName = TargetPinForExecutable->Name;
        } else {
            TSharedPtr<FBlueprintPin> MainInputExec = ExecutableNode->GetExecutionInputPin();
            if (MainInputExec.IsValid()) {
                ActualTargetPinName = MainInputExec->Name;
            }
            UE_LOG(LogPathTracer, Log, TEXT("HandleGloballyProcessedNode: TargetPinForExecutable was null for %s. Inferred entry to main exec pin: %s"), *NodeName, *ActualTargetPinName);
        }

        bool bIsForLoopWithBreak = (SimpleMacroNameForCheck == TEXT("ForLoopWithBreak"));
        bool bIsForEachLoopWithBreak = (SimpleMacroNameForCheck == TEXT("ForEachLoopWithBreak"));
        bool bIsBreakableLoop = bIsForLoopWithBreak || bIsForEachLoopWithBreak;

        if (TargetPinForExecutable.IsValid() && TargetPinForExecutable->IsInput()) {
            if (ActualTargetPinName == TEXT("Break") && bIsBreakableLoop) {
                Message = FString::Printf(TEXT("[Triggers Break on %s (%s) -> Break Pin]"), *NodeName, *NodeGuidShort);
            }
            else if ((ActualTargetPinName == TEXT("Execute") || ActualTargetPinName == TEXT("Exec")) && bIsBreakableLoop) {
                Message = FString::Printf(TEXT("[Loops back to start of %s (%s) -> %s Pin]"), *NodeName, *NodeGuidShort, *ActualTargetPinName);
            }
            else if (SimpleMacroNameForCheck == TEXT("DoN") && ActualTargetPinName == TEXT("Reset")) { 
                Message = FString::Printf(TEXT("[Triggers Reset on %s (%s) -> Reset Pin]"), *NodeName, *NodeGuidShort);
            }
            else if (SimpleMacroNameForCheck == TEXT("DoOnce") && ActualTargetPinName == TEXT("Reset")) { 
                Message = FString::Printf(TEXT("[Triggers Reset on %s (%s) -> Reset Pin]"), *NodeName, *NodeGuidShort);
            }
            else if (SimpleMacroNameForCheck == TEXT("Gate") || ExecutableNode->NodeType == TEXT("K2Node_Gate")) { 
                if (ActualTargetPinName == TEXT("Open")) {
                    Message = FString::Printf(TEXT("[Opens %s (%s) -> Open Pin]"), *NodeName, *NodeGuidShort);
                } else if (ActualTargetPinName == TEXT("Close")) {
                    Message = FString::Printf(TEXT("[Closes %s (%s) -> Close Pin]"), *NodeName, *NodeGuidShort);
                } else if (ActualTargetPinName == TEXT("Toggle")) {
                    Message = FString::Printf(TEXT("[Toggles %s (%s) -> Toggle Pin]"), *NodeName, *NodeGuidShort);
                } else if (ActualTargetPinName == TEXT("Enter")) {
                     Message = FString::Printf(TEXT("[Path enters %s (%s) -> Enter Pin]"), *NodeName, *NodeGuidShort);
                }
            }
            else if (SimpleMacroNameForCheck == TEXT("MultiGate") || ExecutableNode->NodeType == TEXT("K2Node_MultiGate")) { 
                if (ActualTargetPinName == TEXT("Reset")) {
                    Message = FString::Printf(TEXT("[Resets %s (%s) -> Reset Pin]"), *NodeName, *NodeGuidShort);
                } else if (ActualTargetPinName == TEXT("execute")) { 
                    Message = FString::Printf(TEXT("[Path enters %s (%s) -> execute Pin]"), *NodeName, *NodeGuidShort);
                }
                else if (ActualTargetPinName.StartsWith(TEXT("In")) && ActualTargetPinName.Len() > 2 && FChar::IsDigit(ActualTargetPinName[2])) {
                     Message = FString::Printf(TEXT("[Path enters %s (%s) -> %s Pin]"), *NodeName, *NodeGuidShort, *ActualTargetPinName);
                }
            }
        } else if (!TargetPinForExecutable.IsValid()) { 
             if (bIsBreakableLoop && (ActualTargetPinName == TEXT("Execute") || ActualTargetPinName == TEXT("Exec"))) {
                Message = FString::Printf(TEXT("[Loops back to start of %s (%s) -> %s Pin]"), *NodeName, *NodeGuidShort, *ActualTargetPinName);
            } else if ((SimpleMacroNameForCheck == TEXT("Gate") || ExecutableNode->NodeType == TEXT("K2Node_Gate")) && ActualTargetPinName == TEXT("Enter")){
                Message = FString::Printf(TEXT("[Path enters %s (%s) -> %s Pin]"), *NodeName, *NodeGuidShort, *ActualTargetPinName);
            } else if ((SimpleMacroNameForCheck == TEXT("MultiGate") || ExecutableNode->NodeType == TEXT("K2Node_MultiGate")) && ActualTargetPinName == TEXT("execute")){
                Message = FString::Printf(TEXT("[Path enters %s (%s) -> %s Pin]"), *NodeName, *NodeGuidShort, *ActualTargetPinName);
            } else if ((SimpleMacroNameForCheck == TEXT("DoOnce") || ExecutableNode->NodeType == TEXT("K2Node_DoOnce")) && (ActualTargetPinName == TEXT("execute") || ActualTargetPinName == TEXT("In"))){ 
                 Message = FString::Printf(TEXT("[Path enters %s (%s) -> %s Pin]"), *NodeName, *NodeGuidShort, *ActualTargetPinName);
            }
            else if ((SimpleMacroNameForCheck == TEXT("DoN") || ExecutableNode->NodeType == TEXT("K2Node_DoN")) && (ActualTargetPinName == TEXT("Enter"))){ 
                 Message = FString::Printf(TEXT("[Path enters %s (%s) -> %s Pin]"), *NodeName, *NodeGuidShort, *ActualTargetPinName);
            }
        }

        if (Message.IsEmpty()) {
            // GENERALIZED: Reuse existing node formatting logic
            FString OperationDescription = TEXT("Unknown Operation");
    
            if (ExecutableNode.IsValid()) {
                TSet<FString> TempVisitedSet;
                OperationDescription = FMarkdownNodeFormatter::FormatNodeDescription(
                    ExecutableNode,
                    TOptional<FCapturedEventData>(),
                    DataTracerRef,
                    AllNodes,  // Now available!
                    TempVisitedSet,
                    false,
                    false,
                    PathTracerCurrentBlueprintContext
                );
        
                // Clean up formatting
                OperationDescription.ReplaceInline(TEXT("**"), TEXT(""));
                OperationDescription.ReplaceInline(TEXT("`"), TEXT(""));
                OperationDescription = OperationDescription.TrimStartAndEnd();
                if (OperationDescription.StartsWith(TEXT("* "))) {
                    OperationDescription.RightChopInline(2);
                }
            }
    
            Message = FString::Printf(TEXT("[Continue execution at: %s → Previously detailed]"), 
                *OperationDescription);
        }
        UE_LOG(LogPathTracer, Log, TEXT("%s%s - Path to %s (%s) Pin: %s. Context: '%s'. SimpleMacroNameForCheck: '%s'"),
            *CurrentIndentPrefix,
            Message.StartsWith(TEXT("[")) ? TEXT("CONTINUING/TRIGGERING") : TEXT("INFO"),
            *NodeName,
            *NodeGuidShort,
            *ActualTargetPinName,
            *PathTracerCurrentBlueprintContext,
            *SimpleMacroNameForCheck
        );

        if (!CurrentIndentPrefix.IsEmpty() || !Message.IsEmpty()) {
            FString LineToAdd = GenerateMarkdownLine(Message, CurrentIndentPrefix);
            if (OutLines.IsEmpty() || OutLines.Last() != LineToAdd) { OutLines.Add(LineToAdd); }
        }
        
        if (ProcessedInCurrentPath.Contains(CurrentGuid)) ProcessedInCurrentPath.Remove(CurrentGuid);
        if (bAddedExecutableToPath && ProcessedInCurrentPath.Contains(ExecutableGuid)) ProcessedInCurrentPath.Remove(ExecutableGuid);
        return true; 
    }
    return false; 
}


// REPLACE the existing ProcessNodeFormattingAndBranching with this cleaned version
void FMarkdownPathTracer::ProcessNodeFormattingAndBranching(
    TSharedPtr<const FBlueprintNode> ExecutableNode,
    const FString& ExecutableGuid,
    const TOptional<FCapturedEventData>& CurrentCapturedData,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
    TSet<FString>& InOutProcessedGlobally,
    TSet<FString>& ProcessedInCurrentPath,
    const FString& CurrentIndentPrefix,
    bool bIsLastSegment,
    TArray<FString>& OutLines,
    const FString& CurrentBlueprintContext,
    const FTraceStepContext& StepContext 
) {
    bool bHandledExplicitlyInBranch = false;

	if (ExecutableNode->NodeType == TEXT("MacroInstance")) {
		// --- Explicit Macro Handling ---
		const FString* MacroPathPtr = ExecutableNode->RawProperties.Find(TEXT("MacroGraphReference"));
		FString FullMacroPath = MacroPathPtr ? **MacroPathPtr : TEXT("");
		FString SimpleMacroName = MarkdownTracerUtils::ExtractSimpleNameFromPath(FullMacroPath, CurrentBlueprintContext);

        if (SimpleMacroName.Contains(TEXT("ForEachLoop")) || SimpleMacroName.Contains(TEXT("ForLoop")) || SimpleMacroName == TEXT("IsValid")) {
            bHandledExplicitlyInBranch = true;
            TArray<TTuple<TSharedPtr<const FBlueprintPin>, FString>> BranchesToProcess;
            if (SimpleMacroName.Contains(TEXT("ForEachLoop")) || SimpleMacroName.Contains(TEXT("ForLoop"))) {
                BranchesToProcess.Add(MakeTuple(ExecutableNode->GetPin(TEXT("LoopBody")), TEXT("LoopBody:")));
                BranchesToProcess.Add(MakeTuple(ExecutableNode->GetPin(TEXT("Completed")), TEXT("Completed:")));
            } else if (SimpleMacroName == TEXT("IsValid")) {
                BranchesToProcess.Add(MakeTuple(ExecutableNode->GetPin(TEXT("Is Valid")), TEXT("Is Valid:")));
                BranchesToProcess.Add(MakeTuple(ExecutableNode->GetPin(TEXT("Is Not Valid")), TEXT("Is Not Valid:")));
            }
            TArray<TTuple<TSharedPtr<const FBlueprintPin>, FString>> ValidLinkedBranches;
            for(const auto& BranchTuple : BranchesToProcess) {
                if(BranchTuple.Get<0>().IsValid() && BranchTuple.Get<0>()->LinkedPins.Num() > 0) {
                    ValidLinkedBranches.Add(BranchTuple);
                }
            }

            if (ValidLinkedBranches.Num() > 0) {
                FString ChildIndentBase = CalculateNextPrefix(CurrentIndentPrefix, bIsLastSegment);

                for (int32 i = 0; i < ValidLinkedBranches.Num(); ++i) {
                    const TSharedPtr<const FBlueprintPin>& BranchPinToFollow = ValidLinkedBranches[i].Get<0>();
                    const FString& ExplicitBranchLabel = ValidLinkedBranches[i].Get<1>();
                    bool bIsThisBranchLast = (i == ValidLinkedBranches.Num() - 1);
                    
                    // Pure Markdown branching
                    FString BranchConnector = bIsThisBranchLast ? BranchLast : BranchJoin;
                    OutLines.Add(ChildIndentBase + BranchConnector + ExplicitBranchLabel);

                    TSharedPtr<const FBlueprintNode> TargetNode = nullptr;
                    TSharedPtr<const FBlueprintPin> EntryPinOnTargetNode = nullptr;
                    if (BranchPinToFollow->LinkedPins.Num() > 0 && BranchPinToFollow->LinkedPins[0].IsValid()) {
                        EntryPinOnTargetNode = BranchPinToFollow->LinkedPins[0];
                        TargetNode = AllNodes.FindRef(EntryPinOnTargetNode->NodeGuid);
                    }

                    if (TargetNode) {
                        TSet<FString> FreshBranchPath = ProcessedInCurrentPath;
                        TracePathRecursive(TargetNode, TOptional<FCapturedEventData>(), AllNodes, InOutProcessedGlobally, FreshBranchPath,
                                           ChildIndentBase + (bIsThisBranchLast ? IndentSpace : LineCont),
                                           bIsThisBranchLast,
                                           OutLines, CurrentBlueprintContext, 
                                           FTraceStepContext(EntryPinOnTargetNode));
                    } else { 
                        FString Message = FString::Printf(TEXT("[Link outside selection for branch '%s']"), *(BranchPinToFollow->Name));
                        OutLines.Add(GenerateMarkdownLine(Message, ChildIndentBase + (bIsThisBranchLast ? IndentSpace : LineCont)));
                    }
                }
            } else {
                FString Message = FString::Printf(TEXT("[Path ends after macro '%s' - no linked exec outputs]"), *SimpleMacroName);
                OutLines.Add(GenerateMarkdownLine(Message, CalculateNextPrefix(CurrentIndentPrefix, bIsLastSegment)));
            }
        }
	}

   if (!bHandledExplicitlyInBranch) {
    TArray<TSharedPtr<const FBlueprintPin>> LinkedExecOutputs;
    for (const auto& Pair : ExecutableNode->Pins) {
        if (Pair.Value.IsValid() && Pair.Value->IsOutput() && Pair.Value->IsExecution() && Pair.Value->LinkedPins.Num() > 0) {
            LinkedExecOutputs.Add(Pair.Value);
        }
    }
    
    // Sort logic (keep existing sort from current code)
    LinkedExecOutputs.Sort([](const TSharedPtr<const FBlueprintPin>& A, const TSharedPtr<const FBlueprintPin>& B) {
        if (!A.IsValid() || !B.IsValid()) return !A.IsValid();
        const FString NameA = A->FriendlyName.IsEmpty() ? (A->Name.IsEmpty() ? TEXT("") : A->Name) : A->FriendlyName;
        const FString NameB = B->FriendlyName.IsEmpty() ? (B->Name.IsEmpty() ? TEXT("") : B->Name) : B->FriendlyName;
        if (NameA.IsEmpty() && NameB.IsEmpty()) return false;
        if (NameA.IsEmpty()) return false;
        if (NameB.IsEmpty()) return true;
        if (NameA == TEXT("Loop Body") && NameB == TEXT("Completed")) return true;
        if (NameA == TEXT("Completed") && NameB == TEXT("Loop Body")) return false;
        if (NameA == TEXT("True") && NameB == TEXT("False")) return true;
        if (NameA == TEXT("False") && NameB == TEXT("True")) return false;
        if (NameA == TEXT("then") && NameB != TEXT("then")) return true;
        if (NameB == TEXT("then") && NameA != TEXT("then")) return false;
        if (NameA == TEXT("Update") && NameB == TEXT("Finished")) return true;
        if (NameA == TEXT("Finished") && NameB == TEXT("Update")) return false;
        if (NameA == TEXT("Is Valid") && NameB == TEXT("Is Not Valid")) return true;
        if (NameA == TEXT("Is Not Valid") && NameB == TEXT("Is Valid")) return false;
        bool bIsANumeric = NameA.StartsWith(TEXT("Then ")) || NameA.StartsWith(TEXT("Out "));
        bool bIsBNumeric = NameB.StartsWith(TEXT("Then ")) || NameB.StartsWith(TEXT("Out "));
        if (bIsANumeric && bIsBNumeric) {
            bool bASamePrefix = NameA.StartsWith(TEXT("Then ")) == NameB.StartsWith(TEXT("Then "));
            if (bASamePrefix) {
                FString NumStrA = NameA.Mid(NameA.StartsWith(TEXT("Then ")) ? 5 : 4);
                FString NumStrB = NameB.Mid(NameB.StartsWith(TEXT("Then ")) ? 5 : 4);
                if (NumStrA.IsNumeric() && NumStrB.IsNumeric()) {
                    return FCString::Atoi(*NumStrA) < FCString::Atoi(*NumStrB);
                }
            }
        }
        return NameA < NameB;
    });

    int32 NumBranches = LinkedExecOutputs.Num();

    if (NumBranches == 0) {
        if (ExecutableNode->NodeType != TEXT("FunctionResult") && ExecutableNode->NodeType != TEXT("Tunnel")) {
            FString Message = TEXT("[Path ends]");
            OutLines.Add(GenerateMarkdownLine(Message, CalculateNextPrefix(CurrentIndentPrefix, bIsLastSegment)));
        }
    } else { 
        FString ChildIndentBase = CalculateNextPrefix(CurrentIndentPrefix, bIsLastSegment);

        for (int32 i = 0; i < NumBranches; ++i) {
            const TSharedPtr<const FBlueprintPin>& PinToFollow = LinkedExecOutputs[i];
            bool bIsThisBranchLast = (i == NumBranches - 1); 
            
            FString BranchPinName = PinToFollow->FriendlyName.IsEmpty() ? PinToFollow->Name : PinToFollow->FriendlyName;
            FString BranchLabelText = BranchPinName;
            bool bShouldPrintBranchLabel = true;

            if (NumBranches == 1) {
                if (BranchPinName.IsEmpty() || BranchPinName.TrimStartAndEnd().IsEmpty() || 
                    BranchPinName.Equals(TEXT("then"), ESearchCase::IgnoreCase) ||
                    BranchPinName.Equals(TEXT("execute"), ESearchCase::IgnoreCase) ||
                    BranchPinName.Equals(TEXT("Completed"), ESearchCase::IgnoreCase) ||
                    BranchPinName.Equals(TEXT("Finished"), ESearchCase::IgnoreCase) ||
                    BranchPinName.Equals(TEXT("Update"), ESearchCase::IgnoreCase) ||
                    BranchPinName.Equals(TEXT("Output"), ESearchCase::IgnoreCase) ||
                    BranchPinName.Equals(TEXT("Result"), ESearchCase::IgnoreCase)) {
                    bShouldPrintBranchLabel = false;
                }
            }
            if (bShouldPrintBranchLabel && !BranchLabelText.EndsWith(TEXT(":"))) { 
                BranchLabelText += TEXT(":");
            }
            
            // Pure Markdown branch prefix logic
            FString MarkdownBranchPrefix = ChildIndentBase;
            FString NextMarkdownIndent = ChildIndentBase;

            if (NumBranches > 1) {
                MarkdownBranchPrefix += (bIsThisBranchLast ? BranchLast : BranchJoin);
                NextMarkdownIndent += (bIsThisBranchLast ? IndentSpace : LineCont);
            } else if (bShouldPrintBranchLabel) {
                MarkdownBranchPrefix += ExecPrefix;
                NextMarkdownIndent = ChildIndentBase + IndentSpace;
            }

            // Print branch label if needed
            if(bShouldPrintBranchLabel) {
                OutLines.Add(MarkdownBranchPrefix + BranchLabelText);
            }
             
            // Process branch content
            TSharedPtr<const FBlueprintNode> TargetNodeForBranch = nullptr; 
            FString OutsideLinkGuid = TEXT(""); 
            bool bLinkGoesOutside = false;
            TSharedPtr<const FBlueprintPin> EntryPinOnTargetNodeForBranch = nullptr;

            if (PinToFollow->LinkedPins.Num() > 0 && PinToFollow->LinkedPins[0].IsValid()) {
                EntryPinOnTargetNodeForBranch = PinToFollow->LinkedPins[0]; 
                if(EntryPinOnTargetNodeForBranch) { 
                    const TSharedPtr<FBlueprintNode>* TargetNodePtr = AllNodes.Find(EntryPinOnTargetNodeForBranch->NodeGuid);
                    if (TargetNodePtr && TargetNodePtr->IsValid()) { TargetNodeForBranch = *TargetNodePtr; }
                    else { bLinkGoesOutside = true; OutsideLinkGuid = EntryPinOnTargetNodeForBranch->NodeGuid; } 
                }
            }

            if (TargetNodeForBranch) {
                TSet<FString> PathSetToUse = ProcessedInCurrentPath; 
                
                TracePathRecursive(
                    TargetNodeForBranch, 
                    TOptional<FCapturedEventData>(), 
                    AllNodes, 
                    InOutProcessedGlobally, 
                    PathSetToUse, 
                    NextMarkdownIndent, 
                    true,
                    OutLines, 
                    CurrentBlueprintContext, 
                    FTraceStepContext(EntryPinOnTargetNodeForBranch)
                );
            } else { 
                FString Message = bLinkGoesOutside ? FString::Printf(TEXT("[Link outside selection -> %s]"), *(OutsideLinkGuid.Left(8)))
                                                  : FString::Printf(TEXT("[Path ends - Pin '%s' unlinked]"), *(PinToFollow->Name));
                OutLines.Add(GenerateMarkdownLine(Message, NextMarkdownIndent));
            }
        }
    }
}
}

FString FMarkdownPathTracer::SanitizeAnchorName(const FString& InputName)
{
    FString Anchor = InputName;
    Anchor = Anchor.ToLower();
    Anchor.ReplaceInline(TEXT("."), TEXT("-"));
    Anchor.ReplaceInline(TEXT(" "), TEXT("-"));
    Anchor.ReplaceInline(TEXT("_"), TEXT("-"));

    FString Output;
    Output.Reserve(Anchor.Len());
    for (TCHAR Char : Anchor)
    {
        if (FChar::IsAlnum(Char) || Char == TCHAR('-'))
        {
            Output.AppendChar(Char);
        }
    }

    FString TempConsolidated = Output;
    while (TempConsolidated.ReplaceInline(TEXT("--"), TEXT("-"))); 
    Output = TempConsolidated;

    if (Output.StartsWith(TEXT("-"))) { Output.RightChopInline(1); }
    if (Output.EndsWith(TEXT("-"))) { Output.LeftChopInline(1); }
    
    if (!Output.IsEmpty() && (FChar::IsDigit(Output[0]) || Output[0] == TCHAR('-')))
    {
        Output = TEXT("bp-") + Output;
    }
    else if (Output.IsEmpty())
    {
        Output = TEXT("bp-anchor");
    }
    return Output;
}