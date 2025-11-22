/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Structs.cpp


#include "NodeTraceHandlers_Structs.h"
#include "Trace/MarkdownDataTracer.h" // Include main tracer for helpers
#include "Models/BlueprintNode.h"    // Include node/pin models
#include "Models/BlueprintPin.h"
#include "Logging/LogMacros.h"       // For UE_LOG
#include "Internationalization/Regex.h" // For potential string checks
#include "Trace/Utils/MarkdownFormattingUtils.h"
#include "Trace/Utils/MarkdownTracerUtils.h"
#include "Logging/BP2AILog.h" 

//----------------------------------------------------------------//
// Struct Handlers
//----------------------------------------------------------------//

// In NodeTraceHandlers_Structs.cpp
#include "NodeTraceHandlers_Structs.h"
// ... other includes ...
#include "Logging/BP2AILog.h"

FString FNodeTraceHandlers_Structs::HandleBreakStruct(
    TSharedPtr<const FBlueprintNode> Node,
    TSharedPtr<const FBlueprintPin> OutputPin, // This is the struct member pin we are resolving
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
    check(Node.IsValid() && Node->NodeType == TEXT("BreakStruct"));
    check(Tracer); check(OutputPin.IsValid());

    UE_LOG(LogDataTracer, Error, TEXT("  HandleBreakStruct: Node='%s' (GUID:%s), MemberOutputPin='%s' (ID:%s). CtxRecv:'%s', Symbolic:%d, Depth:%d"),
        *Node->Name, *Node->Guid.Left(8),
        *OutputPin->Name, *OutputPin->Id.Left(8),
        *CurrentBlueprintContext, bSymbolicTrace, Depth);
    // 🔧 NEW DEBUG: Show what's connected to ALL input pins of Break node
    UE_LOG(LogDataTracer, Error, TEXT("🔧 BREAK NODE ALL INPUTS:"));
    for (const auto& PinPair : Node->Pins)
    {
        const TSharedPtr<FBlueprintPin>& Pin = PinPair.Value;
        if (Pin.IsValid() && Pin->IsInput())
        {
            UE_LOG(LogDataTracer, Error, TEXT("🔧   Input Pin: '%s', SourcePinFor.Num=%d"), *Pin->Name, Pin->SourcePinFor.Num());
            for (int32 i = 0; i < Pin->SourcePinFor.Num(); ++i)
            {
                if (Pin->SourcePinFor[i].IsValid())
                {
                    FString SourceNodeGuid = Pin->SourcePinFor[i]->NodeGuid;
                    TSharedPtr<FBlueprintNode> SourceNode = CurrentNodesMap.FindRef(SourceNodeGuid);
                    FString SourceNodeName = SourceNode.IsValid() ? SourceNode->Name : TEXT("UNKNOWN");
                    UE_LOG(LogDataTracer, Error, TEXT("🔧     Connected to: NodeGUID='%s', NodeName='%s', PinName='%s'"), 
                        *SourceNodeGuid.Left(8), *SourceNodeName, *Pin->SourcePinFor[i]->Name);
                }
            }
        }
    }

    
    TSharedPtr<const FBlueprintPin> InputStructPin;
    for (const auto& Pair : Node->Pins) {
        const TSharedPtr<const FBlueprintPin>& Pin = Pair.Value;
        // Heuristic: The input struct pin is an input, category "struct", and not an execution pin.
        if (Pin.IsValid() && Pin->IsInput() && Pin->Category == TEXT("struct") && !Pin->IsExecution()) {
            InputStructPin = Pin;
            break;
        }
    }

    if (!InputStructPin.IsValid()) {
        UE_LOG(LogDataTracer, Error, TEXT("    HandleBreakStruct: Could not find InputStructPin on node %s"), *Node->Guid.Left(8));
        return FMarkdownSpan::Error(TEXT("[BreakStruct Input Missing]"));
    }

    UE_LOG(LogDataTracer, Warning, TEXT("    HandleBreakStruct: Tracing InputStructPin '%s' (ID:%s). DefObj:'%s', Links:%d. CtxToPass:'%s', SymbolicForThisTrace:%d"),
        *InputStructPin->Name, *InputStructPin->Id.Left(8), *InputStructPin->DefaultObject, InputStructPin->SourcePinFor.Num(), *CurrentBlueprintContext, bSymbolicTrace);

    // When tracing the input struct, we want its symbolic representation, so use the passed 'bSymbolicTrace'
    FString InputStructValue = Tracer->ResolvePinValueRecursive(InputStructPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap, bSymbolicTrace, CurrentBlueprintContext);
    UE_LOG(LogDataTracer, Warning, TEXT("    HandleBreakStruct: InputStructPin '%s' resolved to: %s"), *InputStructPin->Name, *InputStructValue);

    FString DisplayMemberName = (!OutputPin->FriendlyName.IsEmpty() && OutputPin->FriendlyName != OutputPin->Name) ? OutputPin->FriendlyName : OutputPin->Name;
    DisplayMemberName.TrimStartAndEndInline();
    FString MemberNameSpan = FMarkdownSpan::PinName(FString::Printf(TEXT("%s"), *DisplayMemberName));

    // Check if InputStructValue is already wrapped in (...) for complex expressions or is a simple variable.
    // A simple variable is typically `VarName` or `` `Var Name With Spaces` `` (after FMarkdownSpan::Variable)
    // A complex expression (like another function call) might already be (FuncCall).Pin
    // The desired output is (StructSource).Member or Var.Member
    FString Result;
    if (InputStructValue.StartsWith(TEXT("(")) && InputStructValue.EndsWith(TEXT(")"))) {
        // It's already a parenthesized expression, e.g. (DT[Row]) or (AnotherFunc().ReturnValue)
        Result = FString::Printf(TEXT("%s.%s"), *InputStructValue, *MemberNameSpan);
    } else {
        // It's likely a simple variable or literal, wrap it if it's not a single identifier with backticks
        static const FRegexPattern SimpleVarOrLiteralPattern(TEXT("^`?[a-zA-Z0-9_ ]+`?$"));
        FRegexMatcher VarMatcher(SimpleVarOrLiteralPattern, InputStructValue);
        if (VarMatcher.FindNext()) { // It's a simple variable like `MyVar` or simple literal 'Text'
            Result = FString::Printf(TEXT("%s.%s"), *InputStructValue, *MemberNameSpan);
        } else { // It's some other more complex form that isn't yet parenthesized, so wrap it.
            Result = FString::Printf(TEXT("(%s).%s"), *InputStructValue, *MemberNameSpan);
        }
    }

    UE_LOG(LogDataTracer, Error, TEXT("  HandleBreakStruct: Node %s, OutputPin '%s'. Final Result: %s"), *Node->Guid.Left(8), *OutputPin->Name, *Result);
    return Result;
}
//----------------------------------------------------------------//

FString FNodeTraceHandlers_Structs::HandleMakeStruct(
    TSharedPtr<const FBlueprintNode> Node,
    TSharedPtr<const FBlueprintPin> OutputPin, // The struct output pin
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
    check(Node.IsValid() && Node->NodeType == TEXT("MakeStruct"));
    check(Tracer); check(OutputPin.IsValid());
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleMakeStruct: Processing node %s"), *Node->Guid);

    // Get the struct type name using public helper
    const FString* StructTypePath = Node->RawProperties.Find(TEXT("StructType"));
    FString StructTypeName = StructTypePath ? MarkdownTracerUtils::ExtractSimpleNameFromPath(*StructTypePath) : TEXT("Struct");

    // Format arguments using the Tracer's public helper function
    // Exclude the output pin itself if it somehow appears in inputs
    TSet<FName> Exclusions;
    if (OutputPin.IsValid()) Exclusions.Add(FName(*OutputPin->Name));
    FString ArgsStr = MarkdownFormattingUtils::FormatArgumentsForTrace(Node, Tracer, CurrentNodesMap, Depth + 1, VisitedPins, Exclusions, CallingNode, OuterNodesMap,bSymbolicTrace);

    FString Result = FString::Printf(TEXT("Make<%s>(%s)"), *FMarkdownSpan::DataType(FString::Printf(TEXT("%s"), *StructTypeName)), *ArgsStr); 
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleMakeStruct: Returning %s"), *Result);
    return Result;
}

//----------------------------------------------------------------//

FString FNodeTraceHandlers_Structs::HandleSetFieldsInStruct(
    TSharedPtr<const FBlueprintNode> Node,
    TSharedPtr<const FBlueprintPin> OutputPin, // The struct output pin
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
    check(Node.IsValid() && Node->NodeType == TEXT("SetFieldsInStruct"));
    check(Tracer); check(OutputPin.IsValid());
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleSetFieldsInStruct: Processing node %s"), *Node->Guid);

    // Find the input struct pin (usually 'StructRef')
    TSharedPtr<const FBlueprintPin> InputStructPin = Node->GetPin(TEXT("StructRef"));
    if (!InputStructPin.IsValid()) InputStructPin = Node->GetPin(TEXT("Struct In")); // Fallback name

    FString InputStructValue = InputStructPin.IsValid() ? Tracer->ResolvePinValueRecursive(InputStructPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap) : FMarkdownSpan::Error(TEXT("[?Struct?]"));

    // Get the struct type name from the output pin (usually most reliable)
    FString StructTypeName = MarkdownTracerUtils::ExtractSimpleNameFromPath(OutputPin->SubCategoryObject);
    if (StructTypeName.IsEmpty()) StructTypeName = TEXT("Struct");

    // Format field arguments, excluding the input struct pin itself
    TSet<FName> Exclusions;
    if(InputStructPin.IsValid()) Exclusions.Add(FName(*InputStructPin->Name));
    // Also exclude the output pin itself
    if(OutputPin.IsValid()) Exclusions.Add(FName(*OutputPin->Name));

    FString ArgsStr = MarkdownFormattingUtils::FormatArgumentsForTrace(Node, Tracer, CurrentNodesMap, Depth + 1, VisitedPins, Exclusions, CallingNode, OuterNodesMap, bSymbolicTrace);

    // Return a representation showing modification
    FString Result = FMarkdownSpan::Info(TEXT("Modified"))
                     + FString::Printf(TEXT("<%s>(%s with %s)"),
                        *FMarkdownSpan::DataType(FString::Printf(TEXT("%s"), *StructTypeName)),
                        *InputStructValue,
                        *ArgsStr
                      );

    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleSetFieldsInStruct: Returning %s"), *Result);
    return Result;
}
//----------------------------------------------------------------//