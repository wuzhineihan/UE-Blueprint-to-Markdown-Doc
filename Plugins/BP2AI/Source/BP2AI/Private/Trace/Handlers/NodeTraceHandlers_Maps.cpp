/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Maps.cpp
#include "NodeTraceHandlers_Maps.h"
#include "Trace/MarkdownDataTracer.h"
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Logging/LogMacros.h"
#include "Trace/Utils/MarkdownFormattingUtils.h" // For spans
#include "Logging/BP2AILog.h"

FString FNodeTraceHandlers_Maps::HandleMakeMap(
	TSharedPtr<const FBlueprintNode> Node,
	TSharedPtr<const FBlueprintPin> OutputPin,
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
	check(Node.IsValid() && Node->NodeType == TEXT("MakeMap"));
	check(Tracer);
	check(OutputPin.IsValid());
	UE_LOG(LogDataTracer, Verbose, TEXT("  HandleMakeMap: Processing node %s"), *Node->Guid);

	TArray<FString> PairStrings;
	int32 Index = 0;
	while (true)
	{
		FString KeyPinName = FString::Printf(TEXT("[%d].Key"), Index); // Example Key pin name pattern
		FString ValuePinName = FString::Printf(TEXT("[%d].Value"), Index); // Example Value pin name pattern

		TSharedPtr<const FBlueprintPin> KeyPin = Node->GetPin(KeyPinName);
		TSharedPtr<const FBlueprintPin> ValuePin = Node->GetPin(ValuePinName);

		// Fallback names if standard pattern fails (less common)
		if (!KeyPin.IsValid()) KeyPin = Node->GetPin(FString::Printf(TEXT("Key %d"), Index));
		if (!ValuePin.IsValid()) ValuePin = Node->GetPin(FString::Printf(TEXT("Value %d"), Index));

		if (!KeyPin.IsValid() || !ValuePin.IsValid())
		{
			// Also check for single Key/Value pins if Index is 0 (MakeMap with 1 element)
			if (Index == 0) {
				KeyPin = Node->GetPin(TEXT("Key"));
				ValuePin = Node->GetPin(TEXT("Value"));
				if (!KeyPin.IsValid() || !ValuePin.IsValid()) {
					break; // No more pairs found
				}
			} else {
				break; // No more numbered pairs found
			}
		}

		// Trace key and value recursively
		FString KeyStr = Tracer->ResolvePinValueRecursive(KeyPin, AllNodes, Depth + 1, VisitedPins);
		FString ValueStr = Tracer->ResolvePinValueRecursive(ValuePin, AllNodes, Depth + 1, VisitedPins);

		PairStrings.Add(FString::Printf(TEXT("%s%s%s"), *KeyStr, *FMarkdownSpan::Operator(TEXT(":")), *ValueStr));
		Index++;
	}

	// Format as {Key1: Value1, Key2: Value2}
	FString Result = FMarkdownSpan::LiteralContainer(TEXT("{"))
				   + FString::Join(PairStrings, TEXT(", "))
				   + FMarkdownSpan::LiteralContainer(TEXT("}"));

	UE_LOG(LogDataTracer, Verbose, TEXT("  HandleMakeMap: Returning '%s'"), *Result);
	return Result;
}

FString FNodeTraceHandlers_Maps::HandleMapFind(
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
	check(Node.IsValid() && Node->NodeType == TEXT("MapFind"));
	check(Tracer);
	check(OutputPin.IsValid());
	UE_LOG(LogDataTracer, Verbose, TEXT("  HandleMapFind: Processing node %s"), *Node->Guid);

	TSharedPtr<const FBlueprintPin> MapInputPin = Node->GetPin(TEXT("Map"));
	TSharedPtr<const FBlueprintPin> KeyInputPin = Node->GetPin(TEXT("Key"));

	if (!MapInputPin.IsValid() || !KeyInputPin.IsValid())
	{
		UE_LOG(LogDataTracer, Error, TEXT("  HandleMapFind: Missing Map or Key pin"));
		return FString();
	}

	FString MapTargetValue = Tracer->ResolvePinValueRecursive(MapInputPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap);
	FString KeyValue = Tracer->ResolvePinValueRecursive(KeyInputPin, CurrentNodesMap, Depth + 1, VisitedPins, CallingNode, OuterNodesMap);

	// Format as Map[Key]
	FString Result = FString::Printf(TEXT("%s%s%s"), *MapTargetValue, *FMarkdownSpan::Operator(TEXT("[")), *KeyValue);
	Result += FMarkdownSpan::Operator(TEXT("]"));

	UE_LOG(LogDataTracer, Verbose, TEXT("  HandleMapFind: Returning '%s'"), *Result);
	return Result;
}