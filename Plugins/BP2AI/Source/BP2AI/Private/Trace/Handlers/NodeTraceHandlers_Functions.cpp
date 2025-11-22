/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */


// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Functions.cpp
#include "NodeTraceHandlers_Functions.h"
#include "Trace/MarkdownDataTracer.h" 
#include "Models/BlueprintNode.h"     
#include "Models/BlueprintPin.h"
#include "Logging/LogMacros.h"        
#include "Internationalization/Regex.h" 
#include "Trace/Utils/MarkdownFormattingUtils.h"
#include "Trace/Utils/MarkdownTracerUtils.h"
#include "Kismet2/BlueprintEditorUtils.h" 
#include "Trace/FMarkdownPathTracer.h"
#include "Logging/BP2AILog.h"
#include "Trace/Utils/MarkdownSpanSystem.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"

namespace { // Anonymous namespace for local helper
    FString ConstructFinalSymbolicString(const FString& BaseCall, TSharedPtr<const FBlueprintNode> Node, TSharedPtr<const FBlueprintPin> OutputPin)
    {
        // THE CORRECT RULE:
        // Omit the pin name from the output ONLY if it's the conventional "ReturnValue".
        // This is the simplest, most robust logic. A pure function with a single
        // output named "NewParam" should be treated like a function with multiple outputs,
        // requiring the pin name for clarity.
        if (OutputPin->Name == TEXT("ReturnValue"))
        {
            return BaseCall; // e.g., "MyFunction(Args)"
        }
        
        // For ALL OTHER named output pins (e.g., "NewParam", "Hour", "Minutes"),
        // we wrap the base call in parentheses and append the pin name. This ensures
        // the value being accessed is always explicit.
        return FString::Printf(TEXT("(%s).%s"), *BaseCall, *FMarkdownSpan::PinName(FString::Printf(TEXT("%s"), *OutputPin->Name)));
    }

    bool IsUserDefinedFunction(const FString& ClassPath, const FString& FunctionName)
    {
        UClass* OwnerClass = FindObject<UClass>(nullptr, *ClassPath);
        if (!OwnerClass) 
        {
            return false; // Fallback to existing path-based logic
        }
        
        UFunction* Function = OwnerClass->FindFunctionByName(FName(*FunctionName));
        if (!Function) 
        {
            return false; // Fallback to existing path-based logic
        }
        
        // Same Blueprint ownership logic as Helper_CheckCallFunctionType
        UObject* FunctionActualOuter = Function->GetOuter();
        UBlueprint* FunctionDefiningBP = Cast<UBlueprint>(FunctionActualOuter);
        if (!FunctionDefiningBP && FunctionActualOuter && FunctionActualOuter->IsA<UBlueprintGeneratedClass>()) 
        {
            FunctionDefiningBP = Cast<UBlueprint>(Cast<UBlueprintGeneratedClass>(FunctionActualOuter)->ClassGeneratedBy);
        }
        
        // If a Blueprint defines this function, it's user-defined
        return FunctionDefiningBP != nullptr;
    }

    
}
//----------------------------------------------------------------//
// Function Call Handlers & Helpers
//----------------------------------------------------------------//

// Located in Private/Trace/Handlers/NodeTraceHandlers_Functions.cpp
FString FNodeTraceHandlers_Functions::FormatPureFunctionCall_Internal(
    TSharedPtr<const FBlueprintNode> Node, 
    TSharedPtr<const FBlueprintPin> OutputPin,
    FMarkdownDataTracer* Tracer,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
    int32 Depth,
    TSet<FString>& VisitedPins,
    const FBlueprintDataExtractor& DataExtractor, 
    TSharedPtr<const FBlueprintNode> CurrentCallingNode, 
    const TMap<FString, TSharedPtr<FBlueprintNode>>* CurrentOuterNodesMap, 
    bool bSymbolicTrace,
    const FString& CurrentBlueprintContext
)
{
    check(Tracer);
    check(Node.IsValid() && (Node->NodeType == TEXT("CallFunction") || Node->NodeType == TEXT("CallParentFunction")));
    check(OutputPin.IsValid());

    UE_LOG(LogDataTracer, Verbose, TEXT("    FormatPureFunctionCall_Internal: Node %s (%s), Pin %s, Symbolic=%d, Context='%s'"), *Node->Name, *Node->Guid, *OutputPin->Name, bSymbolicTrace, *CurrentBlueprintContext);

    const FString* FuncNamePtr = Node->RawProperties.Find(TEXT("FunctionName"));
    if (!FuncNamePtr && Node->NodeType == TEXT("CallParentFunction")) { 
        FuncNamePtr = Node->RawProperties.Find(TEXT("SuperFunctionName")); 
    }
    const FString FuncName = FuncNamePtr ? **FuncNamePtr : TEXT("PureFunc");

    TSharedPtr<const FBlueprintPin> TargetPin; 
    for(const auto& Pair : Node->Pins) { if(Pair.Value.IsValid() && Pair.Value->IsInput() && (Pair.Value->Name == TEXT("self") || Pair.Value->Name == TEXT("Target"))) { TargetPin = Pair.Value; break; } }
    
    // TraceTargetPin now returns TEXT("") if the prefix should be omitted.
    FString TargetStr = Tracer->TraceTargetPin(TargetPin, CurrentNodesMap, 0, VisitedPins, CurrentBlueprintContext); 

    FString CallPrefix = TEXT("");
    // If TargetStr is not empty and not 'self', it's a target that needs to be prefixed (and is already formatted).
    if (!TargetStr.IsEmpty() && TargetStr != FMarkdownSpan::Variable(TEXT("self"))) 
    {
        CallPrefix = TargetStr + TEXT(".");
        UE_LOG(LogDataTracer, Verbose, TEXT("      FormatPureFunctionCall_Internal: Using TargetStr '%s' as CallPrefix for FuncName '%s'"), *TargetStr, *FuncName);
    } else
    {
         UE_LOG(LogDataTracer, Verbose, TEXT("      FormatPureFunctionCall_Internal: TargetStr is empty or 'self' for FuncName '%s'. Omitting prefix."), *FuncName);
    }
    // The KnownStaticBlueprintLibrariesSet check is no longer needed here.
    
    TSet<FName> Exclusions; if (TargetPin.IsValid()) { Exclusions.Add(FName(*TargetPin->Name)); }
    
    FString ArgsStr = MarkdownFormattingUtils::FormatArgumentsForTrace(Node, Tracer, CurrentNodesMap, Depth + 1, VisitedPins, Exclusions, CurrentCallingNode, CurrentOuterNodesMap, bSymbolicTrace, CurrentBlueprintContext); 

    FString FuncNameSpan = FMarkdownSpan::FunctionName(FString::Printf(TEXT("%s"), *FuncName));
    FString BaseCall = FString::Printf(TEXT("%s%s(%s)"), *CallPrefix, *FuncNameSpan, *ArgsStr);

    // Special handling for common pure functions
    if (FuncName == TEXT("MakeLiteralGameplayTagContainer") && OutputPin->Name == TEXT("ReturnValue"))
    {
        TSharedPtr<const FBlueprintPin> ValuePin = Node->GetPin(TEXT("Value")); 
        if (ValuePin.IsValid()) {
            FString ContainerValueStr = Tracer->ResolvePinValueRecursive(ValuePin, CurrentNodesMap, Depth + 1, VisitedPins, CurrentCallingNode, CurrentOuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);
            UE_LOG(LogDataTracer, Log, TEXT("      FormatPureFunctionCall_Internal (MakeLiteralGameplayTagContainer): Returning directly traced input 'Value' pin string: '%s'"), *ContainerValueStr);
            return ContainerValueStr; 
        } else { 
            UE_LOG(LogDataTracer, Warning, TEXT("      FormatPureFunctionCall_Internal (MakeLiteralGameplayTagContainer): 'Value' input pin not found. Falling back to BaseCall."));
            return BaseCall; 
        }
    }
    if (FuncName == TEXT("Concat_StrStr") || Node->Name == TEXT("Append"))
    {
        TArray<FString> InputValues; TArray<TSharedPtr<const FBlueprintPin>> InputPinsForConcat;
        for (const auto& Pair : Node->Pins) { if (Pair.Value.IsValid() && Pair.Value->IsInput() && !Pair.Value->IsExecution() && !Pair.Value->IsHidden()) { InputPinsForConcat.Add(Pair.Value); } }
        InputPinsForConcat.Sort([](const auto& PinA, const auto& PinB) { return PinA->Name < PinB->Name; });
        for (const auto& Pin : InputPinsForConcat) {
            InputValues.Add(Tracer->ResolvePinValueRecursive(Pin, CurrentNodesMap, Depth + 1, VisitedPins, CurrentCallingNode, CurrentOuterNodesMap, bSymbolicTrace, CurrentBlueprintContext));
        }
        if (InputValues.Num() > 0) { FString PlusOperator = FMarkdownSpan::Operator(Tracer->GetMathOperatorMap().FindRef(TEXT("Add"))); FString JoinedValues = FString::Join(InputValues, *FString::Printf(TEXT(" %s "), *PlusOperator)); return (InputValues.Num() > 1) ? FString::Printf(TEXT("(%s)"), *JoinedValues) : InputValues[0]; }
        else { return FMarkdownSpan::FunctionName(TEXT("Concat")) + TEXT("()"); }
    }
    if (FuncName == TEXT("Len")) {
        TSharedPtr<const FBlueprintPin> StrPin = Node->GetPin(TEXT("S")); if(!StrPin.IsValid()) StrPin = Node->GetPin(TEXT("SourceString"));
        FString StrValue = StrPin.IsValid() ? Tracer->ResolvePinValueRecursive(StrPin, CurrentNodesMap, Depth + 1, VisitedPins, CurrentCallingNode, CurrentOuterNodesMap, bSymbolicTrace, CurrentBlueprintContext) : FMarkdownSpan::Error(TEXT("?String?"));
        return FMarkdownSpan::FunctionName(TEXT("Len")) + FString::Printf(TEXT("(%s)"), *StrValue);
    }
    if (FuncName.StartsWith(TEXT("Select")) &&
        ( FuncName.EndsWith(TEXT("Float")) || FuncName.EndsWith(TEXT("String")) || FuncName.EndsWith(TEXT("Object")) ||
          FuncName.EndsWith(TEXT("Vector")) || FuncName.EndsWith(TEXT("Rotator")) || FuncName.EndsWith(TEXT("Name")) ||
          FuncName.EndsWith(TEXT("Transform")) || FuncName.EndsWith(TEXT("Color")) || FuncName.EndsWith(TEXT("Class")) ||
          FuncName.EndsWith(TEXT("Int")) 
        )
       )
    {
       TSharedPtr<const FBlueprintPin> PinA = Node->GetPin(TEXT("A"));
       TSharedPtr<const FBlueprintPin> PinB = Node->GetPin(TEXT("B"));
        TSharedPtr<const FBlueprintPin> PinCond = Node->GetPin(TEXT("bPickA"));
        if (!PinCond.IsValid()) { PinCond = Node->GetPin(TEXT("Pick A")); } 
        if (!PinCond.IsValid()) { PinCond = Node->GetPin(TEXT("Condition")); } 
        if (!PinCond.IsValid()) { PinCond = Node->GetPin(TEXT("Index")); } 

        if (PinA.IsValid() && PinB.IsValid() && PinCond.IsValid()) {
           FString ValueA = Tracer->ResolvePinValueRecursive(PinA, CurrentNodesMap, Depth + 1, VisitedPins, CurrentCallingNode, CurrentOuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);
           FString ValueB = Tracer->ResolvePinValueRecursive(PinB, CurrentNodesMap, Depth + 1, VisitedPins, CurrentCallingNode, CurrentOuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);
           FString ValueCond = Tracer->ResolvePinValueRecursive(PinCond, CurrentNodesMap, Depth + 1, VisitedPins, CurrentCallingNode, CurrentOuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);
           FString TernaryResult = FString::Printf(TEXT("(%s %s %s %s %s)"), *ValueCond, *FMarkdownSpan::Operator(TEXT("?")), *ValueA, *FMarkdownSpan::Operator(TEXT(":")), *ValueB);
           return TernaryResult;
       } 
    }

    // Default formatting if no special case matched above
    if (!OutputPin.IsValid() || OutputPin->Name == TEXT("ReturnValue") || OutputPin->Name.IsEmpty()) { return BaseCall; }
    else { return FString::Printf(TEXT("(%s).%s"), *BaseCall, *FMarkdownSpan::PinName(FString::Printf(TEXT("%s"), *OutputPin->Name))); }
}

FString FNodeTraceHandlers_Functions::FormatPureMacroCall_Internal(
    TSharedPtr<const FBlueprintNode> Node, 
    TSharedPtr<const FBlueprintPin> OutputPin,
    FMarkdownDataTracer* Tracer,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
    int32 Depth,
    TSet<FString>& VisitedPins,
    const FBlueprintDataExtractor& DataExtractor,
    TSharedPtr<const FBlueprintNode> CurrentCallingNode, 
    const TMap<FString, TSharedPtr<FBlueprintNode>>* CurrentOuterNodesMap, 
    bool bSymbolicTrace,
    const FString& CurrentBlueprintContext 
)
{
    check(Tracer);
    check(Node.IsValid() && Node->NodeType == TEXT("MacroInstance"));
    check(OutputPin.IsValid());

    UE_LOG(LogDataTracer, Verbose, TEXT("    FormatPureMacroCall_Internal: Node %s (%s), Pin %s, Symbolic=%d, Context='%s'"), *Node->Name, *Node->Guid, *OutputPin->Name, bSymbolicTrace, *CurrentBlueprintContext);

    const FString* MacroPathPtr = Node->RawProperties.Find(TEXT("MacroGraphReference"));
    FString SimpleMacroName = MacroPathPtr ? MarkdownTracerUtils::ExtractSimpleNameFromPath(**MacroPathPtr, CurrentBlueprintContext) : TEXT("PureMacro"); 
    if (SimpleMacroName.IsEmpty()) SimpleMacroName = TEXT("PureMacro");

    UE_LOG(LogDataTracer, Verbose, TEXT("      FormatPureMacroCall_Internal: Default formatting for pure macro %s. Context: '%s'"), *SimpleMacroName, *CurrentBlueprintContext);
    TSet<FName> Exclusions; 
    FString ArgsStr = MarkdownFormattingUtils::FormatArgumentsForTrace(Node, Tracer, CurrentNodesMap, Depth + 1, VisitedPins, Exclusions, CurrentCallingNode, CurrentOuterNodesMap, bSymbolicTrace, CurrentBlueprintContext); 

    FString MacroNameSpan = FMarkdownSpan::MacroName(FString::Printf(TEXT("`%s"), *SimpleMacroName));
    FString BaseCall = FString::Printf(TEXT("%s(%s)"), *MacroNameSpan, *ArgsStr);
    
    TSharedPtr<const FBlueprintPin> PrimaryOutputOnNode; 
    for(const auto& Pair : Node->Pins) {
        if(Pair.Value.IsValid() && Pair.Value->IsOutput() && !Pair.Value->IsExecution() && !Pair.Value->IsHidden()) {
            PrimaryOutputOnNode = Pair.Value;
            break;
        }
    }

    if (!OutputPin.IsValid() || OutputPin == PrimaryOutputOnNode || OutputPin->Name.IsEmpty() || (PrimaryOutputOnNode.IsValid() && OutputPin->Name == PrimaryOutputOnNode->Name) ) {
        return BaseCall;
    } else {
        return FString::Printf(TEXT("(%s).%s"), *BaseCall, *FMarkdownSpan::PinName(FString::Printf(TEXT("%s"), *OutputPin->Name)));
    }
}



FString FNodeTraceHandlers_Functions::HandleCallFunction(
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
    check(Node.IsValid() && (Node->NodeType == TEXT("CallFunction") || Node->NodeType == TEXT("CallParentFunction")));
    check(Tracer);
    check(OutputPin.IsValid());

    // --- 1. Get Basic Function Info ---
    const FString* FuncNamePtr = Node->RawProperties.Find(TEXT("FunctionName"));
    if (!FuncNamePtr && Node->NodeType == TEXT("CallParentFunction")) {
        FuncNamePtr = Node->RawProperties.Find(TEXT("SuperFunctionName"));
    }
    const FString FuncName = FuncNamePtr ? **FuncNamePtr : TEXT("UnknownFunction");

    // --- 2. Handle Standard Library Functions first ---
    // (This part is simplified as FormatPureFunctionCall_Internal already handles many of these)
    // --- 2. Handle Type Conversions → Function Formatting ---
    if (FuncName.StartsWith(TEXT("Conv_")) || FuncName.StartsWith(TEXT("To")))
    {
        return FormatPureFunctionCall_Internal(Node, OutputPin, Tracer, CurrentNodesMap, Depth, VisitedPins, DataExtractor, CallingNode, OuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);
    }

    // --- 3. Handle Math/String/Text Operators → Operator Formatting ---
    FString BaseOperationName = FuncName.Left(FuncName.Find(TEXT("_")));
    if (Tracer->GetMathOperatorMap().Contains(BaseOperationName) || Tracer->GetMathOperatorMap().Contains(FuncName))
    {
        UE_LOG(LogDataTracer, Verbose, TEXT("  HandleCallFunction: Function '%s' detected as operator. Routing to FormatOperator."), *FuncName);
    
        // *** ADD THIS: Check if it's a unary operator ***
        if (FuncName == TEXT("Not_PreBool") || BaseOperationName == TEXT("Not"))
        {
            return MarkdownFormattingUtils::FormatUnaryOperator(Node, OutputPin, Tracer, CurrentNodesMap, Depth, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);
        }
    
        // Otherwise use binary operator formatting
        return MarkdownFormattingUtils::FormatOperator(Node, OutputPin, Tracer, CurrentNodesMap, Depth, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);
    }
    
    // --- 3. Determine if it's a linkable user function ---
    bool bShouldGenerateSymbolicLink = false;
    FString AnchorKey = FuncName;
    FString LinkDisplayName = FuncName;
    const FString* ParentClassPathFromProps = Node->RawProperties.Find(TEXT("FunctionParentClassPath"));

    if (ParentClassPathFromProps && !ParentClassPathFromProps->IsEmpty())
    {
        // First apply existing path-based exclusions
        if (!ParentClassPathFromProps->StartsWith(TEXT("/Script/")) && 
            !ParentClassPathFromProps->StartsWith(TEXT("/Engine/")))
        {
            // Add new: Blueprint ownership check
            if (IsUserDefinedFunction(*ParentClassPathFromProps, FuncName))
            {
                bShouldGenerateSymbolicLink = true;
            
                // ... rest of existing anchor generation code stays the same
                FString DefiningAssetPath = **ParentClassPathFromProps;
                UClass* TempClass = FindObject<UClass>(nullptr, *DefiningAssetPath);
                if (TempClass && TempClass->ClassGeneratedBy && TempClass->ClassGeneratedBy->IsA<UBlueprint>())
                {
                    DefiningAssetPath = TempClass->ClassGeneratedBy->GetPathName();
                }
                FString BlueprintNameForHint = FPaths::GetBaseFilename(DefiningAssetPath);
                if (BlueprintNameForHint.EndsWith(TEXT("_C")))
                {
                    BlueprintNameForHint.LeftChopInline(2);
                }

                FString ReconstructedUniqueHint = FString::Printf(TEXT("%s.%s"), *BlueprintNameForHint, *FuncName);
                AnchorKey = FMarkdownPathTracer::SanitizeAnchorName(ReconstructedUniqueHint);
                LinkDisplayName = FuncName;
            }
            // If IsUserDefinedFunction returns false, bShouldGenerateSymbolicLink stays false
        }
        // If path starts with /Script/ or /Engine/, bShouldGenerateSymbolicLink stays false
    }

    // --- 4. Get Target and Arguments ---
    TSharedPtr<const FBlueprintPin> TargetPin;
    for (const auto& Pair : Node->Pins) {
        if (Pair.Value.IsValid() && Pair.Value->IsInput() && (Pair.Value->Name == TEXT("self") || Pair.Value->Name == TEXT("Target"))) {
            TargetPin = Pair.Value;
            break;
        }
    }
    FString TargetStr = Tracer->TraceTargetPin(TargetPin, CurrentNodesMap, Depth + 1, VisitedPins, CurrentBlueprintContext);
    
    TSet<FName> Exclusions;
    if (TargetPin.IsValid()) Exclusions.Add(FName(*TargetPin->Name));
    FString ArgsStr = MarkdownFormattingUtils::FormatArgumentsForTrace(Node, Tracer, CurrentNodesMap, Depth + 1, VisitedPins, Exclusions, CallingNode, OuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);
    
    // --- 5. Construct the Base Call (Name + Arguments) ---
    FString CallableName;
    if (bShouldGenerateSymbolicLink)
    {
        // First, format the text that will go inside the link using the established FMarkdownSpan wrapper.
        FString FormattedLinkText = FMarkdownSpan::FunctionName(LinkDisplayName);

        // Now, check the context and build the link tag manually, just like the working code in FMarkdownPathTracer.
        if (FMarkdownSpanSystem::GetCurrentContext().IsHTML())
        {
            // In HTML mode, build the <a> tag.
            // Note: The text inside the tag is already formatted by FMarkdownSpan, so it might contain spans. We don't escape it again.
            CallableName = FString::Printf(TEXT("<a href=\"#%s\" class=\"graph-link function-link\">%s</a>"), *AnchorKey, *FormattedLinkText);
        }
        else // RawMarkdown or CleanText
        {
            // In Markdown mode, build the [](#) link.
            CallableName = FString::Printf(TEXT("[%s](#%s)"), *FormattedLinkText, *AnchorKey);
        }
    }
    else
    {
        // This is a native function, so it's not a link. Just format the name.
        CallableName = FMarkdownSpan::FunctionName(LinkDisplayName);
    }

    
    FString BaseCall = CallableName + FString::Printf(TEXT("(%s)"), *ArgsStr);
    
    // --- 6. Prepend Target Object ---
    if (!TargetStr.IsEmpty() && TargetStr != FMarkdownSpan::Variable(TEXT("self")))
    {
        BaseCall = TargetStr + TEXT(".") + BaseCall;
    }
    
    // --- 7. Construct Final String with Pin Name using the new helper ---
    // The helper now correctly handles all cases (single return, multi-return, named return).
    // CRITICAL: We no longer wrap the result in FMarkdownSpan::Info() here, as the helper now produces the complete, final format.
    return ConstructFinalSymbolicString(BaseCall, Node, OutputPin);
}




// PROVIDE FULL FUNCTION
FString FNodeTraceHandlers_Functions::HandleCallMacro(
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
    check(Node.IsValid() && Node->NodeType == TEXT("MacroInstance"));
    check(Tracer); 
    check(OutputPin.IsValid());
    
    bool bIsPureMacro = Node->IsPure();

    const FString* MacroPathPtr = Node->RawProperties.Find(TEXT("MacroGraphReference"));
    FString FullMacroPath = MacroPathPtr ? **MacroPathPtr : FString(TEXT(""));
    
    FString SimpleMacroName = MarkdownTracerUtils::ExtractSimpleNameFromPath(FullMacroPath, CurrentBlueprintContext);
    if (SimpleMacroName.IsEmpty()) SimpleMacroName = Node->Name; 
    if (SimpleMacroName.IsEmpty()) SimpleMacroName = TEXT("MacroInstance");

    UE_LOG(LogDataTracer, Log, TEXT("HandleCallMacro ENTRY: NodeName='%s', SimpleMacroName='%s', OutputPinName='%s', SymbolicTrace=%d, IsPureMacro=%d, FullMacroPath='%s', Context='%s'"), 
        *Node->Name, *SimpleMacroName, *OutputPin->Name, bSymbolicTrace, bIsPureMacro, *FullMacroPath, *CurrentBlueprintContext);
                                   
    bool bIsStandardEngineMacro = !FullMacroPath.IsEmpty() && 
                                  FullMacroPath.Contains(TEXT("/StandardMacros.StandardMacros"));

    bool bIsUserGraph = !FullMacroPath.IsEmpty() && !bIsStandardEngineMacro;
    UE_LOG(LogDataTracer, Error, TEXT("HandleCallMacro DETAIL: Node='%s', MacroPath='%s', IsUserGraph=%s, IsPureNode=%s. QueuePtr=%p, ProcessedPathsPtr=%p"),
    *Node->Name, *FullMacroPath, bIsUserGraph ? TEXT("true") : TEXT("false"), Node->IsPure() ? TEXT("true") : TEXT("false"),
    Tracer->CurrentGraphsToDefineSeparatelyPtr, Tracer->CurrentProcessedSeparateGraphPathsPtr);

    
    if (bIsUserGraph && Tracer->CurrentGraphsToDefineSeparatelyPtr && Tracer->CurrentProcessedSeparateGraphPathsPtr) {
    UE_LOG(LogDataTracer, Error, TEXT("HandleCallMacro: Condition (bIsUserGraph && QueuePtr && ProcessedPathsPtr) MET. FullMacroPath for check: '%s'"), *FullMacroPath);

    bool bAlreadyProcessedPath = Tracer->CurrentProcessedSeparateGraphPathsPtr->Contains(FullMacroPath);
    UE_LOG(LogDataTracer, Error, TEXT("HandleCallMacro: Check 1: AlreadyProcessedPath ('%s' in ProcessedPathsSet) = %s"), *FullMacroPath, bAlreadyProcessedPath ? TEXT("true") : TEXT("false"));

    if (!bAlreadyProcessedPath) {
        FString NameForQueueHead = MarkdownTracerUtils::ExtractSimpleNameFromPath(FullMacroPath, TEXT(""));
        TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType> TupleToAdd(NameForQueueHead, FullMacroPath, FMarkdownPathTracer::EUserGraphType::Macro);
        
        bool bTupleAlreadyInQueue = Tracer->CurrentGraphsToDefineSeparatelyPtr->Contains(TupleToAdd);
        UE_LOG(LogDataTracer, Error, TEXT("HandleCallMacro: Check 2: TupleAlreadyInQueue ('%s':'%s' in Queue) = %s"), *NameForQueueHead, *FullMacroPath, bTupleAlreadyInQueue ? TEXT("true") : TEXT("false"));

        if (!bTupleAlreadyInQueue)
        {
            UE_LOG(LogDataTracer, Error, TEXT("HandleCallMacro: >>> ADDING TO QUEUE (via DataTracer): Hint='%s', Path='%s', Type=Macro. Current Queue Size BEFORE add: %d"),
                *NameForQueueHead, *FullMacroPath, Tracer->CurrentGraphsToDefineSeparatelyPtr->Num());
            Tracer->CurrentGraphsToDefineSeparatelyPtr->Add(TupleToAdd); // AddUnique not needed if checking Contains
            UE_LOG(LogDataTracer, Error, TEXT("HandleCallMacro: <<< ADDED TO QUEUE. Current Queue Size AFTER add: %d"), Tracer->CurrentGraphsToDefineSeparatelyPtr->Num());
        } else {
            UE_LOG(LogDataTracer, Error, TEXT("HandleCallMacro: SKIPPED ADDING tuple to queue because it was already present."));
        }
    } else {
        UE_LOG(LogDataTracer, Error, TEXT("HandleCallMacro: SKIPPED ADDING to queue because path was already processed in ProcessedPathsSet."));
    }
} else {
    FString Reason = FString::Printf(TEXT("bIsUserGraph=%s, QueuePtr Valid=%s, ProcessedPathsPtr Valid=%s"),
        bIsUserGraph ? TEXT("true") : TEXT("false"),
        Tracer->CurrentGraphsToDefineSeparatelyPtr ? TEXT("YES") : TEXT("NO (NULL)"),
        Tracer->CurrentProcessedSeparateGraphPathsPtr ? TEXT("YES") : TEXT("NO (NULL)")
    );
    UE_LOG(LogDataTracer, Error, TEXT("HandleCallMacro: Main condition for queuing FAILED. Reason: %s"), *Reason);
}
    
    if (bSymbolicTrace && bIsUserGraph) 
    {
        FString MacroDisplayNameStr = SimpleMacroName; 
        if (MacroDisplayNameStr.IsEmpty() || MacroDisplayNameStr == TEXT("MacroInstance")) MacroDisplayNameStr = Node->Name;
        if (MacroDisplayNameStr.IsEmpty()) MacroDisplayNameStr = Node->NodeType;
        
        TSet<FName> Exclusions; 
        FString ArgsStr = MarkdownFormattingUtils::FormatArgumentsForTrace(Node, Tracer, CurrentNodesMap, Depth + 1, VisitedPins, Exclusions, CallingNode, OuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);

        FString OutputPinNameStr = OutputPin->Name.IsEmpty() ? TEXT("ReturnValue") : OutputPin->Name;
        FString CallableNameSpan = FMarkdownSpan::MacroName(FString::Printf(TEXT("%s"), *MacroDisplayNameStr));
        FString PinNameSpan = FMarkdownSpan::PinName(FString::Printf(TEXT("%s"), *OutputPinNameStr));
        FString SymbolicLinkWithArgs = FString::Printf(TEXT("%s.%s (Macro Inputs: %s)"), *CallableNameSpan, *PinNameSpan, *ArgsStr);

        UE_LOG(LogDataTracer, Log, TEXT("HandleCallMacro: RETURNING SYMBOLIC (User Macro with Args) for '%s', OutputPin '%s' -> '%s'"), 
            *SimpleMacroName, *OutputPin->Name, *SymbolicLinkWithArgs);
        return FMarkdownSpan::Info(SymbolicLinkWithArgs);
    }
    
   if (bIsPureMacro)
    {
        // This is the new logic block. It differentiates between standard and user pure macros.
        if (bIsStandardEngineMacro)
        {
            // It's a standard pure macro (e.g., IsValid). Use the original, simple formatting helper.
            UE_LOG(LogDataTracer, Log, TEXT("HandleCallMacro: Detected as PURE STANDARD macro '%s'. Using original formatter."), *SimpleMacroName);
            return FormatPureMacroCall_Internal(Node, OutputPin, Tracer, CurrentNodesMap, Depth, VisitedPins, DataExtractor, CallingNode, OuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);
        }
        else // This is a PURE USER-DEFINED macro. It needs a hyperlink.
        {
            UE_LOG(LogDataTracer, Log, TEXT("HandleCallMacro: Detected as PURE USER macro '%s'. Generating hyperlink."), *SimpleMacroName);

            // Create a temporary path tracer just to get the canonical hint for the anchor.
            // This guarantees the anchor name will match the one used for the definition.
            FString UniqueGraphNameHint;
            FString DiscoveredGraphPath;
            FMarkdownPathTracer PathTracerForHint(*Tracer, const_cast<FBlueprintDataExtractor&>(DataExtractor));
            PathTracerForHint.IsInternalUserGraph(Node, DiscoveredGraphPath, UniqueGraphNameHint);
            if (UniqueGraphNameHint.IsEmpty()) UniqueGraphNameHint = SimpleMacroName; // Fallback

            FString AnchorKey = FMarkdownPathTracer::SanitizeAnchorName(UniqueGraphNameHint);

            // --- START OF SURGICAL FIX ---
            auto [ContextName, ItemName] = MarkdownTracerUtils::FGraphNameNormalizer::SplitContextAndItem(UniqueGraphNameHint);
            FString LinkDisplayName;

            // If the macro's context is different from the current graph's context, show the full qualified name.
            // Otherwise, just show the simple name.
            if (!CurrentBlueprintContext.IsEmpty() && !ContextName.IsEmpty() && !ContextName.Equals(CurrentBlueprintContext, ESearchCase::IgnoreCase))
            {
                // It's from an external library, show the full hint.
                LinkDisplayName = UniqueGraphNameHint;
            }
            else
            {
                // It's a local macro, show the simple name.
                LinkDisplayName = !ItemName.IsEmpty() ? ItemName : UniqueGraphNameHint;
            }
            // --- END OF SURGICAL FIX ---

            TSet<FName> Exclusions;
            FString ArgsStr = MarkdownFormattingUtils::FormatArgumentsForTrace(Node, Tracer, CurrentNodesMap, Depth + 1, VisitedPins, Exclusions, CallingNode, OuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);

            // Build the link in a context-aware way.
            FString CallableName;
            if (FMarkdownSpanSystem::GetCurrentContext().IsHTML())
            {
                FString FormattedLinkText = FMarkdownSpan::MacroName(LinkDisplayName);
                CallableName = FString::Printf(TEXT("<a href=\"#%s\" class=\"graph-link macro-link\">%s</a>"), *AnchorKey, *FormattedLinkText);
            }
            else // RawMarkdown or CleanText
            {
                FString FormattedLinkText = FMarkdownSpan::MacroName(LinkDisplayName);
                CallableName = FString::Printf(TEXT("[%s](#%s)"), *FormattedLinkText, *AnchorKey);
            }
            
            FString BaseCall = CallableName + FString::Printf(TEXT("(%s)"), *ArgsStr);
            return ConstructFinalSymbolicString(BaseCall, Node, OutputPin);
        }
    }
    else // Non-Pure Macro (Executable)
    {
        if (bIsStandardEngineMacro)
        {
            FString ResultStringForStandardMacro;
            const FName OutputPinFName(*OutputPin->Name);
            FString MacroDisplayNameSpan = FMarkdownSpan::MacroName(FString::Printf(TEXT("%s"), *SimpleMacroName)); // Used by several handlers

            // --- Loop Macros ---
            if (SimpleMacroName == TEXT("ForEachLoop") || 
                SimpleMacroName == TEXT("ForEachLoopWithBreak") ||
                SimpleMacroName == TEXT("ReverseForEachLoop"))
            {
                if (OutputPinFName == FName(TEXT("Array Element")) || OutputPinFName == FName(TEXT("ArrayElement"))) {
                    ResultStringForStandardMacro = FMarkdownSpan::Variable(TEXT("`Loop Element`"));
                } else if (OutputPinFName == FName(TEXT("Array Index")) || OutputPinFName == FName(TEXT("ArrayIndex"))) {
                    ResultStringForStandardMacro = FMarkdownSpan::Variable(TEXT("`Loop Index`"));
                }
            }
            else if (SimpleMacroName == TEXT("ForLoop") || SimpleMacroName == TEXT("ForLoopWithBreak"))
            {
                if (OutputPinFName == FName(TEXT("Index"))) {
                    ResultStringForStandardMacro = FMarkdownSpan::Variable(TEXT("`Loop Index`"));
                }
            }
            // --- Math-like Standard Executable Macros ---
            else if (SimpleMacroName == TEXT("IncrementInt") || SimpleMacroName == TEXT("IncrementFloat") ||
                     SimpleMacroName == TEXT("DecrementInt") || SimpleMacroName == TEXT("DecrementFloat"))
            {
                // These macros typically output the modified value on a pin often named "Result" or by the macro name itself
                // if they are modified to have an output pin beyond just exec.
                // For their symbolic representation, we show MacroName(TracedInput).
                TSharedPtr<const FBlueprintPin> MainInputPinToTrace = Node->GetPin(TEXT("Value")); 
                if (MainInputPinToTrace.IsValid()) {
                     FString TracedInput = Tracer->ResolvePinValueRecursive(MainInputPinToTrace, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap, true /*bSymbolicTrace for input*/, CurrentBlueprintContext);
                     ResultStringForStandardMacro = FString::Printf(TEXT("%s(%s)"), *MacroDisplayNameSpan, *TracedInput);
                } else {
                    ResultStringForStandardMacro = FString::Printf(TEXT("%s(?)"), *MacroDisplayNameSpan); // Input pin not found
                }
            }
            else if (SimpleMacroName == TEXT("NegateInt") || SimpleMacroName == TEXT("NegateFloat"))
            {
                TSharedPtr<const FBlueprintPin> MainInputPinToTrace = Node->GetPin(TEXT("Value"));
                if (MainInputPinToTrace.IsValid()) {
                    FString TracedInput = Tracer->ResolvePinValueRecursive(MainInputPinToTrace, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap, true, CurrentBlueprintContext);
                    ResultStringForStandardMacro = FString::Printf(TEXT("%s(%s)"), *MacroDisplayNameSpan, *TracedInput);
                } else {
                    ResultStringForStandardMacro = FString::Printf(TEXT("%s(?)"), *MacroDisplayNameSpan);
                }
            }
            // --- ManipulateInternal (used by Inc/Dec) ---
            else if (SimpleMacroName == TEXT("ManipulateIntInternal") || SimpleMacroName == TEXT("ManipulateFloatInternal"))
            {
                // This macro is internal to Increment/Decrement but if ever traced directly for its "Result" pin
                if (OutputPinFName == FName(TEXT("Result"))) {
                    TSharedPtr<const FBlueprintPin> RefPin = Node->GetPin(SimpleMacroName == TEXT("ManipulateIntInternal") ? TEXT("IntegerRef") : TEXT("FloatRef"));
                    TSharedPtr<const FBlueprintPin> ValuePin = Node->GetPin(TEXT("NewValue")); // Or "Amount" depending on internal macro version
                    if (!ValuePin.IsValid()) ValuePin = Node->GetPin(TEXT("Amount"));

                    if (RefPin.IsValid() && ValuePin.IsValid()) {
                        FString TracedRef = Tracer->ResolvePinValueRecursive(RefPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap, true, CurrentBlueprintContext);
                        FString TracedValue = Tracer->ResolvePinValueRecursive(ValuePin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap, true, CurrentBlueprintContext);
                        ResultStringForStandardMacro = FString::Printf(TEXT("%s(%s=%s, %s=%s)"), 
                            *MacroDisplayNameSpan, 
                            *FMarkdownSpan::ParamName(FString::Printf(TEXT("%s"),*RefPin->Name)), *TracedRef,
                            *FMarkdownSpan::ParamName(FString::Printf(TEXT("%s"),*ValuePin->Name)), *TracedValue
                        );
                    } else {
                        ResultStringForStandardMacro = FString::Printf(TEXT("%s(?, ?)"), *MacroDisplayNameSpan);
                    }
                }
            }
            // --- Other Standard Executable Macros ---
            else if (SimpleMacroName == TEXT("Do N")) 
            {
                 if(OutputPinFName == FName(TEXT("Counter"))) { 
                    ResultStringForStandardMacro = MacroDisplayNameSpan + TEXT(".") + FMarkdownSpan::Variable(TEXT("`Counter`")); 
                 }
            }
            else if (SimpleMacroName == TEXT("FlipFlop"))
            {
                if(OutputPinFName == FName(TEXT("IsA"))) { ResultStringForStandardMacro = MacroDisplayNameSpan + TEXT(".") + FMarkdownSpan::Variable(TEXT("`IsA`")); }
                else if(OutputPinFName == FName(TEXT("IsB"))) { ResultStringForStandardMacro = MacroDisplayNameSpan + TEXT(".") + FMarkdownSpan::Variable(TEXT("`IsB`")); }
            }

            if (!ResultStringForStandardMacro.IsEmpty()) {
                UE_LOG(LogDataTracer, Error, TEXT("HandleCallMacro: Standard Exec Macro '%s': Returning SYMBOLIC output for Pin '%s' -> '%s'"), 
                    *SimpleMacroName, *OutputPin->Name, *ResultStringForStandardMacro);
                return ResultStringForStandardMacro;
            } else {
                 UE_LOG(LogDataTracer, Warning, TEXT("  HandleCallMacro: Standard Exec Macro '%s' (Node: %s) has traceable output pin '%s' but no specific symbolic representation defined above. Falling back to deep trace for value."), 
                    *SimpleMacroName, *Node->Name, *OutputPin->Name);
            }
        }
        
        // Fallback for Non-Pure User Macros OR Unhandled Standard Exec Macros: Deep Trace into definition for actual value
        UE_LOG(LogDataTracer, Log, TEXT("  HandleCallMacro: Fallback for NON-PURE macro '%s' (Node: %s). Attempting deep trace for output pin '%s'. Context: '%s'"), 
            *SimpleMacroName, *Node->Name, *OutputPin->Name, *CurrentBlueprintContext);

        if (FullMacroPath.IsEmpty()) { 
            UE_LOG(LogDataTracer, Error, TEXT("  HandleCallMacro (Deep Trace): Macro Path Missing for '%s'"), *SimpleMacroName);
            return FMarkdownSpan::Error(TEXT("[Macro Path Missing]")); 
        }

        TMap<FString, TSharedPtr<FBlueprintNode>> MacroGraphNodes;
        if (!Tracer->GetDataExtractorRef().ExtractNodesFromGraph(FullMacroPath, MacroGraphNodes)) 
        { 
            UE_LOG(LogDataTracer, Error, TEXT("  HandleCallMacro (Deep Trace): Macro Graph Extraction Failed for '%s' (Path: %s)"), *SimpleMacroName, *FullMacroPath);
            return FMarkdownSpan::Error(FString::Printf(TEXT("[Macro Graph Extraction Failed: %s]"), *SimpleMacroName)); 
        }
        if (MacroGraphNodes.IsEmpty())
        {
            UE_LOG(LogDataTracer, Error, TEXT("  HandleCallMacro (Deep Trace): Macro Graph Empty or Not Found for '%s' (Path: %s)"), *SimpleMacroName, *FullMacroPath);
            return FMarkdownSpan::Error(FString::Printf(TEXT("[Macro Graph Empty or Not Found: %s]"), *SimpleMacroName));
        }

        TSharedPtr<const FBlueprintNode> OutputTunnelNode = nullptr;
        for (const auto& Pair : MacroGraphNodes) {
            if (Pair.Value.IsValid() && Pair.Value->NodeType == TEXT("Tunnel")) {
                TSharedPtr<FBlueprintPin> TunnelInputPinCheck = Pair.Value->GetPin(OutputPin->Name, TEXT("EGPD_Input")); 
                if (TunnelInputPinCheck != nullptr) { 
                    OutputTunnelNode = Pair.Value;
                    break;
                }
            }
        }

        if (!OutputTunnelNode.IsValid()) { 
            UE_LOG(LogDataTracer, Error, TEXT("  HandleCallMacro (Deep Trace): Macro Output Tunnel for Pin '%s' Not Found in '%s' (Path: %s)"), *OutputPin->Name, *SimpleMacroName, *FullMacroPath);
            return FMarkdownSpan::Error(FString::Printf(TEXT("[Macro Output Tunnel for '%s' Not Found in %s]"), *OutputPin->Name, *SimpleMacroName)); 
        }
        
        TSharedPtr<const FBlueprintPin> TunnelInputPinToTrace = OutputTunnelNode->GetPin(OutputPin->Name, TEXT("EGPD_Input"));

        if (!TunnelInputPinToTrace.IsValid()) {
             UE_LOG(LogDataTracer, Error, TEXT("  HandleCallMacro (Deep Trace): Tunnel node '%s' found for output '%s', but could not find its corresponding INPUT pin named '%s' in '%s'"), 
                *OutputTunnelNode->Guid, *OutputPin->Name, *OutputPin->Name, *SimpleMacroName);
             return FMarkdownSpan::Error(FString::Printf(TEXT("[Macro Tunnel Input '%s' Invalid in %s]"), *OutputPin->Name, *SimpleMacroName));
         }

        TSet<FString> MacroVisitedPins; 
        FString MacroAssetContext = MarkdownTracerUtils::ExtractSimpleNameFromPath(FullMacroPath, TEXT("")); 
		if(MacroAssetContext.Contains(TEXT(":"))) MacroAssetContext = MacroAssetContext.Left(MacroAssetContext.Find(TEXT(":")));
        if(MacroAssetContext.IsEmpty()) MacroAssetContext = SimpleMacroName;

        UE_LOG(LogDataTracer, Log, TEXT("    HandleCallMacro (Deep Trace): Tracing into definition of '%s'. TunnelInputPin: '%s'. MacroAssetContext for this trace: '%s'"), 
            *SimpleMacroName, *TunnelInputPinToTrace->Name, *MacroAssetContext);
            
        return Tracer->ResolvePinValueRecursive(TunnelInputPinToTrace, MacroGraphNodes, Depth + 1, MacroVisitedPins, Node, &CurrentNodesMap, false, MacroAssetContext); 
    }
}




FString FNodeTraceHandlers_Functions::HandleFunctionEntryPin(
    TSharedPtr<const FBlueprintNode> Node,
    TSharedPtr<const FBlueprintPin> OutputPinFromEntry,
    FMarkdownDataTracer* Tracer,
    const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap,
    int32 Depth,
    TSet<FString>& VisitedPins,
    const FBlueprintDataExtractor& DataExtractor,
    TSharedPtr<const FBlueprintNode> CallingNode, // Expected to be null
    const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap, // Expected to be null
    bool bSymbolicTrace,
    const FString& CurrentBlueprintContext // Context of this function's graph
)
{
    check(Node.IsValid() && Node->NodeType == TEXT("FunctionEntry"));
    check(OutputPinFromEntry.IsValid() && OutputPinFromEntry->IsOutput());
    check(Tracer);

    UE_LOG(LogDataTracer, Log, TEXT("  HandleFunctionEntryPin: Node='%s', OutputPin='%s'. CurrentContext='%s', Symbolic=%d."),
        *Node->Name, *OutputPinFromEntry->Name, *CurrentBlueprintContext, bSymbolicTrace);

    const TMap<FName, FString>* CallSiteArgs = Tracer->GetCurrentCallsiteArguments();
    FName PinNameFName(*OutputPinFromEntry->Name);

    if (CallSiteArgs != nullptr && CallSiteArgs->Contains(PinNameFName))
    {
        FString ResolvedValueFromCallsite = CallSiteArgs->FindChecked(PinNameFName);
        UE_LOG(LogDataTracer, Log, TEXT("    HandleFunctionEntryPin: Pin '%s' found in CallSiteArgs. Returning: '%s'"),
            *OutputPinFromEntry->Name, *ResolvedValueFromCallsite);
        return ResolvedValueFromCallsite;
    }
    else
    {
        // Fallback: If not in call-site args (e.g., tracing function definition directly),
        // represent as a parameter of the function.
        FString FunctionNameForDisplay = Node->Name; // FunctionEntry node's name is usually the function name
        if (FunctionNameForDisplay.IsEmpty())
        {
            FunctionNameForDisplay = Node->RawProperties.FindRef(TEXT("FunctionReference_MemberName")); // Fallback
        }
         if (FunctionNameForDisplay.IsEmpty()) FunctionNameForDisplay = TEXT("Function");


        UE_LOG(LogDataTracer, Log, TEXT("    HandleFunctionEntryPin: Pin '%s' NOT in CallSiteArgs. Context: '%s'. Returning symbolic 'ValueFrom(FunctionEntry.PinName)' style."),
            *OutputPinFromEntry->Name, *CurrentBlueprintContext);
        
        return FMarkdownSpan::Info(TEXT("ValueFrom")) + 
               FString::Printf(TEXT("(%s.%s)"), 
                    *FMarkdownSpan::FunctionName(FString::Printf(TEXT("%s"),*FunctionNameForDisplay)), // Use the function's name
                    *FMarkdownSpan::PinName(FString::Printf(TEXT("%s"), *OutputPinFromEntry->Name)) // The parameter pin name
                );
    }
}


// Add this to NodeTraceHandlers_Functions.cpp
bool IsNativeEngineFunction(const FString& ClassPath, const FString& FunctionName)
{
    UClass* OwnerClass = FindObject<UClass>(nullptr, *ClassPath);
    if (!OwnerClass) return false; // Fall back to existing path logic
    
    UFunction* Function = OwnerClass->FindFunctionByName(FName(*FunctionName));
    if (!Function) return false; // Fall back to existing path logic
    
    // Same pattern as your existing code - if no Blueprint owns it, it's native
    UObject* FunctionOuter = Function->GetOuter();
    UBlueprint* FunctionDefiningBP = Cast<UBlueprint>(FunctionOuter);
    if (!FunctionDefiningBP && FunctionOuter && FunctionOuter->IsA<UBlueprintGeneratedClass>()) {
        FunctionDefiningBP = Cast<UBlueprint>(Cast<UBlueprintGeneratedClass>(FunctionOuter)->ClassGeneratedBy);
    }
    
    return !FunctionDefiningBP; // If no Blueprint defines it, it's native
}
//----------------------------------------------------------------//