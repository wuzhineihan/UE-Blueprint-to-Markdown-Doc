/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Basic.cpp


#include "NodeTraceHandlers_Basic.h"
#include "Trace/MarkdownDataTracer.h" // Include main tracer for FMarkdownSpan & helpers
#include "Models/BlueprintNode.h"    // Include node/pin models
#include "Models/BlueprintPin.h"
#include "Logging/BP2AILog.h"
#include "Logging/LogMacros.h"       // For UE_LOG
#include "Trace/Utils/MarkdownFormattingUtils.h"
#include "Trace/Utils/MarkdownTracerUtils.h"
#include "Logging/BP2AILog.h"


//----------------------------------------------------------------//
// Basic Handlers
//----------------------------------------------------------------//
FString FNodeTraceHandlers_Basic::HandleVariableGet(
    TSharedPtr<const FBlueprintNode> Node,
    TSharedPtr<const FBlueprintPin> OutputPin,
    FMarkdownDataTracer* Tracer,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
    int32 Depth,
    TSet<FString>& VisitedPins,
    const FBlueprintDataExtractor& DataExtractor,
    TSharedPtr<const FBlueprintNode> CallingNode,
    const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
    bool bSymbolicTrace,
    const FString& CurrentBlueprintContext
)
{
    check(Node.IsValid() && Node->NodeType == TEXT("VariableGet"));
    check(Tracer);
    
    // Get the variable name
    const FString* VarNamePtr = Node->RawProperties.Find(TEXT("VariableName"));
    const FString VarName = VarNamePtr ? **VarNamePtr : TEXT("UnknownVariable");

    // 🆕 NEW: Check for source connection FIRST (store, don't return early)
    FString SourcePrefix = TEXT(""); // Empty by default
    TSharedPtr<const FBlueprintPin> TargetPin = Node->GetPin(TEXT("self"));
    if (TargetPin.IsValid() && TargetPin->IsInput() && TargetPin->SourcePinFor.Num() > 0) {
        TSharedPtr<const FBlueprintPin> SourcePin = TargetPin->SourcePinFor[0];
        if (SourcePin.IsValid()) {
            TSharedPtr<const FBlueprintNode> SourceNode = CurrentNodesMap.FindRef(SourcePin->NodeGuid);
            // 🆕 ONLY apply our logic to actual variable nodes
            if (SourceNode.IsValid() && SourceNode->NodeType == TEXT("VariableGet")) {
                const FString* SourceVarNamePtr = SourceNode->RawProperties.Find(TEXT("VariableName"));
                if (SourceVarNamePtr && !SourceVarNamePtr->IsEmpty()) {
                    SourcePrefix = FString(**SourceVarNamePtr) + TEXT(".");
                    UE_LOG(LogDataTracer, Error, TEXT("  HandleVariableGet: Found source connection '%s'"), *SourcePrefix);
                }
            }
        }
    }

    
    UE_LOG(LogDataTracer, Error, TEXT("  HandleVariableGet: Processing variable '%s'"), *VarName);
    
    // 🔧 NEW: Check for split struct pin patterns in the OutputPin name
    // From logs: Pin names like "Vector2D_X", "MyTransform_Location_X" indicate split pins
    if (OutputPin.IsValid())
    {
        FString PinName = OutputPin->Name;
        UE_LOG(LogDataTracer, Error, TEXT("🔧 SPLIT PIN CHECK: Variable='%s', OutputPin='%s'"), *VarName, *PinName);
        
        // Check if this is a split struct pin (PinName contains variable name + component)
        if (PinName.Contains(VarName + TEXT("_")))
        {
            // Extract the component part after the underscore
            FString ComponentPart;
            if (PinName.Split(VarName + TEXT("_"), nullptr, &ComponentPart))
            {
                // Parse nested components (e.g., "Location_X" -> "Location.X")
                FString ComponentPath = ComponentPart;
                ComponentPath = ComponentPath.Replace(TEXT("_"), TEXT("."));
                
                FString Result = FString::Printf(TEXT("%s.%s"), 
                    *FMarkdownSpan::Variable(SourcePrefix + VarName), 
                    *FMarkdownSpan::PinName(ComponentPath));
                
                UE_LOG(LogDataTracer, Error, TEXT("🔧 SPLIT PIN DETECTED: Variable='%s', Component='%s' -> Result='%s'"), 
                    *VarName, *ComponentPath, *Result);
                return Result;
            }
        }
    }
    
    // 🔧 Check if this variable has an asset reference (Data Asset or Data Table)
    const FString* CDO_DefaultObjectNamePtr = Node->RawProperties.Find(TEXT("CDO_DefaultObjectName"));
    if (CDO_DefaultObjectNamePtr && !CDO_DefaultObjectNamePtr->IsEmpty())
    {
        FString AssetName = *CDO_DefaultObjectNamePtr;
        
        // 🔧 Distinguish between Data Assets and Data Tables
        const FString* IsDataAssetPtr = Node->RawProperties.Find(TEXT("IsDataAssetProperty"));
        bool bIsDataAsset = (IsDataAssetPtr && *IsDataAssetPtr == TEXT("true"));
        
        if (bIsDataAsset)
        {
            // For Data Assets: Show asset name directly (e.g., "DA_AbilityOne")
            UE_LOG(LogDataTracer, Error, TEXT("  HandleVariableGet: Using Data Asset name '%s' for Data Asset variable '%s'"), 
                *AssetName, *VarName);
            return FMarkdownSpan::Variable(SourcePrefix + AssetName);

        }
        else
        {
            // For Data Tables: Show variable name with asset context (e.g., "MYTable(DT_TestStruct)")
            UE_LOG(LogDataTracer, Error, TEXT("  HandleVariableGet: Using Data Table format '%s(%s)' for variable '%s'"), 
                *VarName, *AssetName, *VarName);
            return FString::Printf(TEXT("%s(%s)"), 
                *FMarkdownSpan::Variable(SourcePrefix + VarName), 
                *FMarkdownSpan::LiteralName(AssetName));
        }
    }
    
    // 🔧 Check if this is a struct variable and show struct type information
    // Look for the output pin to get struct type information
    TSharedPtr<const FBlueprintPin> OutputPin_ForStructCheck = Node->GetPin(VarName);
    if (OutputPin_ForStructCheck.IsValid() && 
        OutputPin_ForStructCheck->Category == TEXT("struct") && 
        !OutputPin_ForStructCheck->SubCategoryObject.IsEmpty())
    {
        // Extract clean struct type name from the path
        FString StructTypePath = OutputPin_ForStructCheck->SubCategoryObject;
        FString StructTypeName = StructTypePath;
        
        // Extract just the struct name from the path (e.g., "S_WeaponData" from "/Script/MyProject.S_WeaponData")
        int32 LastDotIndex;
        if (StructTypePath.FindLastChar(TEXT('.'), LastDotIndex))
        {
            StructTypeName = StructTypePath.Mid(LastDotIndex + 1);
        }
        
        // Only show struct type if it's meaningful and different from variable name
        if (!StructTypeName.IsEmpty() && 
            StructTypeName != VarName && 
            StructTypeName != TEXT("Object") && 
            StructTypeName.Len() > 1)
        {
            UE_LOG(LogDataTracer, Error, TEXT("  HandleVariableGet: Using struct format '%s(%s)' for struct variable '%s'"), 
                *VarName, *StructTypeName, *VarName);
            return FString::Printf(TEXT("%s(%s)"), 
                *FMarkdownSpan::Variable(SourcePrefix + VarName), 
                *FMarkdownSpan::LiteralName(StructTypeName));
        }
    }
    
    // Default behavior: Return the variable name (for strings, primitives, etc.)
    UE_LOG(LogDataTracer, Error, TEXT("  HandleVariableGet: Using variable name '%s'"), *VarName);
    return FMarkdownSpan::Variable(SourcePrefix + VarName);
}
//----------------------------------------------------------------//

FString FNodeTraceHandlers_Basic::HandleLiteral(
    TSharedPtr<const FBlueprintNode> Node,
    TSharedPtr<const FBlueprintPin> OutputPin,
    FMarkdownDataTracer* Tracer,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
    int32 Depth,
    TSet<FString>& VisitedPins,
    const FBlueprintDataExtractor& DataExtractor,
    TSharedPtr<const FBlueprintNode> CallingNode,
    const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
    bool bSymbolicTrace,
    const FString& CurrentBlueprintContext
)
{
    check(Node.IsValid() && Node->NodeType == TEXT("Literal"));
    check(OutputPin.IsValid()); // Literal handler relies on the specific output pin
    check(Tracer); // Need the tracer instance to call helpers
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleLiteral: Processing node %s, pin %s"), *Node->Guid, *OutputPin->Name);

    // Delegate formatting to the main tracer's helper function
    FString FormattedValue = MarkdownFormattingUtils::FormatDefaultValue(OutputPin, Tracer);

    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleLiteral: Returning '%s'"), *FormattedValue);
    return FormattedValue;
}

//----------------------------------------------------------------//

FString FNodeTraceHandlers_Basic::HandleSelf(
    TSharedPtr<const FBlueprintNode> Node,
    TSharedPtr<const FBlueprintPin> OutputPin,
    FMarkdownDataTracer* Tracer,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
    int32 Depth,
    TSet<FString>& VisitedPins,
    const FBlueprintDataExtractor& DataExtractor,
    TSharedPtr<const FBlueprintNode> CallingNode,
    const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
    bool bSymbolicTrace,
    const FString& CurrentBlueprintContext
)
{
     // Check both NodeType and UEClass as factory might use either
     check(Node.IsValid() && (Node->NodeType == TEXT("Self") || Node->UEClass == TEXT("/Script/BlueprintGraph.K2Node_Self")));
     check(Tracer);
     UE_LOG(LogDataTracer, Verbose, TEXT("  HandleSelf: Processing node %s"), *Node->Guid);
     FString FormattedValue = FMarkdownSpan::Variable(TEXT("self"));
     UE_LOG(LogDataTracer, Verbose, TEXT("  HandleSelf: Returning '%s'"), *FormattedValue);
     return FormattedValue;
}

//----------------------------------------------------------------//

FString FNodeTraceHandlers_Basic::HandleVariableSet(
    TSharedPtr<const FBlueprintNode> Node,
    TSharedPtr<const FBlueprintPin> OutputPin, // The pin being traced (e.g., Output_Get)
    FMarkdownDataTracer* Tracer,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
    int32 Depth,
    TSet<FString>& VisitedPins,
    const FBlueprintDataExtractor& DataExtractor,
    TSharedPtr<const FBlueprintNode> CallingNode,
    const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
    bool bSymbolicTrace,
    const FString& CurrentBlueprintContext
)
{
    check(Node.IsValid() && Node->NodeType == TEXT("VariableSet"));
    check(Tracer);
    check(OutputPin.IsValid());
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleVariableSet: Processing node %s for output pin %s"), *Node->Guid, *OutputPin->Name);

    // Check if we are tracing the passthrough output pin
    const FString* VarNamePtr = Node->RawProperties.Find(TEXT("VariableName"));
    const FString VarName = VarNamePtr ? **VarNamePtr : FString();
    bool bIsPassthroughPin = OutputPin->Name == TEXT("Output_Get") || (!VarName.IsEmpty() && OutputPin->Name == VarName);

    if (bIsPassthroughPin)
    {
        UE_LOG(LogDataTracer, Verbose, TEXT("  HandleVariableSet: Tracing passthrough pin. Finding input value pin."));
        // Find the input pin corresponding to the variable name
        TSharedPtr<const FBlueprintPin> InputValuePin;
        if (!VarName.IsEmpty()) InputValuePin = Node->GetPin(VarName);
        // Fallback to common names if VariableName lookup failed
        if (!InputValuePin.IsValid()) InputValuePin = Node->GetPin(TEXT("Value"));
        if (!InputValuePin.IsValid()) InputValuePin = Node->GetPin(TEXT("InputPin")); // Less common

        if (InputValuePin.IsValid() && InputValuePin->IsInput())
        {
            UE_LOG(LogDataTracer, Verbose, TEXT("  HandleVariableSet: Found input pin %s. Tracing recursively."), *InputValuePin->Name);
            // Call the PUBLIC recursive helper on the tracer instance
            return Tracer->ResolvePinValueRecursive(InputValuePin, CurrentNodesMap, Depth + 1, VisitedPins);
        }
        else
        {
            UE_LOG(LogDataTracer, Warning, TEXT("  HandleVariableSet: Could not find input value pin for passthrough on node %s"), *Node->Guid);
            return FMarkdownSpan::Error(TEXT("[Set Input Missing]"));
        }
    }
    else
    {
        // Tracing some other output pin (e.g., 'self', or maybe an invalid state)
        UE_LOG(LogDataTracer, Verbose, TEXT("  HandleVariableSet: Tracing non-passthrough output pin '%s'. Returning symbolic info."), *OutputPin->Name);
         // Special case: if tracing 'self', return 'self'
         if (OutputPin->Name == TEXT("self")) return FMarkdownSpan::Variable(TEXT("self"));
         // Otherwise, return generic info
        return FMarkdownSpan::Info(TEXT("ValueFrom")) + FString::Printf(TEXT("(%s.%s)"), *Node->NodeType, *FMarkdownSpan::PinName(FString::Printf(TEXT("%s"), *OutputPin->Name)));
    }
}

// --- File: Private\Trace\Handlers\NodeTraceHandlers_Basic.cpp ---

// Provide the FULL FUNCTION to be replaced.
// Function: FNodeTraceHandlers_Basic::HandleTunnel

FString FNodeTraceHandlers_Basic::HandleTunnel(
    TSharedPtr<const FBlueprintNode> Node,
    TSharedPtr<const FBlueprintPin> PinBeingTraced,
    FMarkdownDataTracer* Tracer,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
    int32 Depth,
    TSet<FString>& VisitedPins,
    const FBlueprintDataExtractor& DataExtractor,
    TSharedPtr<const FBlueprintNode> CallingNode, // Node instance in the *outer* graph that called the graph containing this tunnel
    const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap, // Node map of the *outer* graph
    bool bSymbolicTrace,
    const FString& CurrentBlueprintContext // Context of the graph where THIS tunnel node exists
)
{
    check(Node.IsValid() && Node->NodeType == TEXT("Tunnel"));
    check(Tracer);
    check(PinBeingTraced.IsValid());

    UE_LOG(LogDataTracer, Log, TEXT("  HandleTunnel: Node='%s' (%s), PinBeingTraced='%s' (%s), Direction='%s'. CurrentContext='%s', Symbolic=%d. CallingNode='%s', OuterMapPresent=%s"),
        *Node->Name, *Node->Guid.Left(8), 
        *PinBeingTraced->Name, *PinBeingTraced->Id.Left(8), *PinBeingTraced->Direction,
        *CurrentBlueprintContext, bSymbolicTrace,
        CallingNode.IsValid() ? *CallingNode->Name : TEXT("None"),
        OuterNodesMap ? TEXT("Yes") : TEXT("No"));

    if (PinBeingTraced->IsOutput()) // Pin being traced is an OUTPUT pin of THIS tunnel node (i.e., an INPUT tunnel)
    {
        // This means we are trying to get the value that flows OUT of an INPUT tunnel.
        // This value should come from the call-site arguments if available (e.g., during inline expansion).
        const TMap<FName, FString>* CallSiteArgs = Tracer->GetCurrentCallsiteArguments();
        FName PinNameFName(*PinBeingTraced->Name);

        if (CallSiteArgs != nullptr && CallSiteArgs->Contains(PinNameFName))
        {
            FString ResolvedValueFromCallsite = CallSiteArgs->FindChecked(PinNameFName);
            UE_LOG(LogDataTracer, Log, TEXT("    HandleTunnel (InputTunnel.OutputPin): Pin '%s' found in CallSiteArgs. Returning: '%s'"),
                *PinBeingTraced->Name, *ResolvedValueFromCallsite);
            return ResolvedValueFromCallsite;
        }
        // Fallback: If not in call-site args (e.g. tracing a macro definition directly, not a call),
        // it implies the input tunnel's output pin represents an abstract input to the graph.
        else
        {
            UE_LOG(LogDataTracer, Log, TEXT("    HandleTunnel (InputTunnel.OutputPin): Pin '%s' NOT in CallSiteArgs. Context: '%s'. Returning symbolic 'ValueFrom(Tunnel.PinName)'."),
                *PinBeingTraced->Name, *CurrentBlueprintContext);
            return FMarkdownSpan::Info(TEXT("ValueFrom")) + FString::Printf(TEXT("(Tunnel.%s)"), *FMarkdownSpan::PinName(FString::Printf(TEXT("%s"), *PinBeingTraced->Name)));
        }
    }
    else // PinBeingTraced is an INPUT pin of THIS tunnel node (i.e., an OUTPUT tunnel)
    {
        // This means we are trying to resolve what value flows INTO an OUTPUT tunnel pin from within its own graph.
        UE_LOG(LogDataTracer, Log, TEXT("    HandleTunnel (OutputTunnel.InputPin): Tracing INPUT pin '%s' on Tunnel '%s'. Looking backwards inside current graph ('%s')."), 
            *PinBeingTraced->Name, *Node->Name, *CurrentBlueprintContext);
        if (PinBeingTraced->SourcePinFor.Num() > 0 && PinBeingTraced->SourcePinFor[0].IsValid())
        {
            TSharedPtr<const FBlueprintPin> SourcePinWithinGraph = PinBeingTraced->SourcePinFor[0];
            UE_LOG(LogDataTracer, Log, TEXT("      HandleTunnel: Found internal source pin '%s' on node '%s'. Tracing recursively. Context for RVR: '%s'."),
                *SourcePinWithinGraph->Name, *SourcePinWithinGraph->NodeGuid.Left(8), *CurrentBlueprintContext);
            // Trace backwards within the current graph (CurrentNodesMap)
            return Tracer->ResolvePinValueRecursive(SourcePinWithinGraph, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);
        }
        else
        {
            // This input pin on the output tunnel is not linked from within the graph.
            // It might have a default, or represent an unassigned output.
            UE_LOG(LogDataTracer, Warning, TEXT("    HandleTunnel (OutputTunnel.InputPin): Input pin '%s' on Output Tunnel '%s' has no internal source link. Using default value."), *PinBeingTraced->Name, *Node->Guid.Left(8));
            return MarkdownFormattingUtils::FormatDefaultValue(PinBeingTraced, Tracer);
        }
    }
}


//----------------------------------------------------------------//