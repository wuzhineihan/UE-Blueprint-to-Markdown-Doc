/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Text.cpp
#include "NodeTraceHandlers_Text.h"
#include "Trace/MarkdownDataTracer.h"
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Logging/LogMacros.h"
#include "Trace/Utils/MarkdownFormattingUtils.h" // For spans and FormatArgumentsForTrace
#include "Logging/BP2AILog.h"

FString FNodeTraceHandlers_Text::HandleFormatText(
	TSharedPtr<const FBlueprintNode> Node,
	TSharedPtr<const FBlueprintPin> OutputPin, // The 'Result' output pin
	FMarkdownDataTracer* Tracer,
	const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes,
	int32 Depth,
	TSet<FString>& VisitedPins,
	const FBlueprintDataExtractor& DataExtractor,
	TSharedPtr<const FBlueprintNode> CallingNode,
  const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
  bool bSymbolicTrace,
  const FString& CurrentBlueprintContext
)
{
	check(Node.IsValid() && Node->NodeType == TEXT("FormatText"));
	check(Tracer);
	check(OutputPin.IsValid()); // Should be the 'Result' pin
	UE_LOG(LogDataTracer, Verbose, TEXT("  HandleFormatText: Processing node %s"), *Node->Guid);

	// Find the 'Format' input pin
	TSharedPtr<const FBlueprintPin> FormatPin = Node->GetPin(TEXT("Format"));
	FString FormatStringValue = FormatPin.IsValid() ? Tracer->ResolvePinValueRecursive(FormatPin, AllNodes, Depth + 1, VisitedPins, CallingNode, OuterNodesMap) : FMarkdownSpan::Error(TEXT("[?Format?]"));

	// Format the arguments using the utility function, excluding the 'Format' pin
	TSet<FName> Exclusions;
	if (FormatPin.IsValid())
	{
		Exclusions.Add(FName(*FormatPin->Name));
	}
	FString ArgsStr = MarkdownFormattingUtils::FormatArgumentsForTrace(Node, Tracer, AllNodes, Depth + 1, VisitedPins, Exclusions, CallingNode, OuterNodesMap, bSymbolicTrace);

	// Format the output symbolically
	FString Result = FMarkdownSpan::FunctionName(TEXT("FormatText"))
				   + FString::Printf(TEXT("(Format=%s, Args={%s})"), *FormatStringValue, *ArgsStr);

	UE_LOG(LogDataTracer, Verbose, TEXT("  HandleFormatText: Returning %s"), *Result);
	return Result;
}

FString FNodeTraceHandlers_Text::HandleTextLiteral(
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
	const FString& CurrentBlueprintContext)
{
	check(Node.IsValid() && Node->NodeType == TEXT("TextLiteral"));
	check(Tracer);
	check(OutputPin.IsValid()); // Should be the 'Result' pin
	UE_LOG(LogDataTracer, Verbose, TEXT("  HandleTextLiteral: Processing node %s"), *Node->Guid);

	// Find the 'Text' input pin
	TSharedPtr<const FBlueprintPin> TextPin = Node->GetPin(TEXT("Text"));
	FString TextStringValue = TextPin.IsValid() ? Tracer->ResolvePinValueRecursive(TextPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap) : FMarkdownSpan::Error(TEXT("[?Text?]"));

	// Format the output symbolically
	FString Result = FMarkdownSpan::Keyword(TEXT("TextLiteral"))
				   + FString::Printf(TEXT("(Text=%s)"), *TextStringValue);

	UE_LOG(LogDataTracer, Verbose, TEXT("  HandleTextLiteral: Returning %s"), *Result);
	return Result;
}