/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Enums.cpp


#include "NodeTraceHandlers_Enums.h"
#include "Trace/MarkdownDataTracer.h"
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Logging/BP2AILog.h"
#include "Logging/LogMacros.h"
#include "Trace/Utils/MarkdownFormattingUtils.h"
#include "Trace/Utils/MarkdownTracerUtils.h"
#include "Logging/BP2AILog.h"


//----------------------------------------------------------------//
// Enum Handlers
//----------------------------------------------------------------//

FString FNodeTraceHandlers_Enums::HandleEnumComparison(
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
    check(Node.IsValid()); check(Tracer); check(OutputPin.IsValid());
    check(Node->NodeType == TEXT("EnumEquality") || Node->NodeType == TEXT("EnumInequality"));
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleEnumComparison: Processing node %s (%s)"), *Node->NodeType, *Node->Guid);

    // Find input pins (heuristic: first two non-hidden byte/enum inputs)
    TSharedPtr<const FBlueprintPin> PinA, PinB;
    int FoundPins = 0;
    for(const auto& Pair : Node->Pins)
    {
        if(Pair.Value.IsValid() && Pair.Value->IsInput() && !Pair.Value->IsHidden())
        {
             // Check category is byte or subcategory looks like enum
             if (Pair.Value->Category == TEXT("byte") || (!Pair.Value->SubCategoryObject.IsEmpty() && Pair.Value->SubCategoryObject.Contains(TEXT("Enum"))))
             {
                 if (FoundPins == 0) PinA = Pair.Value;
                 else if (FoundPins == 1) PinB = Pair.Value;
                 FoundPins++;
                 if (FoundPins >= 2) break;
             }
        }
    }

    if (!PinA.IsValid() || !PinB.IsValid()) {
        UE_LOG(LogDataTracer, Warning, TEXT("  HandleEnumComparison: Could not find two input pins for %s"), *Node->Guid);
        return FMarkdownSpan::Error(TEXT("[Enum Comp Inputs Missing]"));
    }

    FString ValueA = Tracer->ResolvePinValueRecursive(PinA, CurrentNodesMap, Depth + 1, VisitedPins);
    FString ValueB = Tracer->ResolvePinValueRecursive(PinB, CurrentNodesMap, Depth + 1, VisitedPins);

    FString OpSymbol = (Node->NodeType == TEXT("EnumEquality")) ? TEXT("==") : TEXT("!=");
    FString Result = FString::Printf(TEXT("(%s %s %s)"), *ValueA, *FMarkdownSpan::Operator(OpSymbol), *ValueB);

    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleEnumComparison: Returning '%s'"), *Result);
    return Result;
}

//----------------------------------------------------------------//

FString FNodeTraceHandlers_Enums::HandleCastByteToEnum(
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
    check(Node.IsValid() && Node->NodeType == TEXT("CastByteToEnum")); check(Tracer); check(OutputPin.IsValid());
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleCastByteToEnum: Processing node %s"), *Node->Guid);

    TSharedPtr<const FBlueprintPin> BytePin = Node->GetPin(TEXT("Byte"));
    if (!BytePin.IsValid()) { UE_LOG(LogDataTracer, Warning, TEXT("  HandleCastByteToEnum: Missing Byte input pin.")); return FMarkdownSpan::Error(TEXT("[CastByteToEnum Input Missing]")); }

    FString ByteValue = Tracer->ResolvePinValueRecursive(BytePin, CurrentNodesMap, Depth + 1, VisitedPins);

    FString EnumTypeName = MarkdownTracerUtils::ExtractSimpleNameFromPath(OutputPin->SubCategoryObject);
    if (EnumTypeName.IsEmpty()) EnumTypeName = TEXT("Enum?");

    FString Result = FMarkdownSpan::Keyword(TEXT("Cast")) + FString::Printf(TEXT("<%s>(%s)"), *FMarkdownSpan::EnumType(FString::Printf(TEXT("%s"), *EnumTypeName)), *ByteValue);
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleCastByteToEnum: Returning '%s'"), *Result);
    return Result;
}


FString FNodeTraceHandlers_Enums::HandleEnumLiteral(
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
    check(Node.IsValid() && Node->NodeType == TEXT("EnumLiteral")); check(Tracer); check(OutputPin.IsValid());
    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleEnumLiteral: Processing node %s for pin %s"), *Node->Guid, *OutputPin->Name);

    // EnumLiterals directly represent their value via the output pin's defaults.
    // We can just use the standard default value formatting logic.
    FString FormattedValue = MarkdownFormattingUtils::FormatDefaultValue(OutputPin, Tracer);

    UE_LOG(LogDataTracer, Verbose, TEXT("  HandleEnumLiteral: Returning formatted default value '%s'"), *FormattedValue);
    return FormattedValue;
}


//----------------------------------------------------------------//

FString FNodeTraceHandlers_Enums::HandleSwitchEnum(
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
    // Implementation of HandleSwitchEnum function
    // This function is not provided in the original file or the code block
    // It's assumed to exist as it's called in the original file
    // Implementation needed
    return FString(); // Placeholder return, actual implementation needed
}
