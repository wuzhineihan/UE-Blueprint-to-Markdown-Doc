/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_FunctionResult.cpp
#include "NodeTraceHandlers_FunctionResult.h"
#include "Trace/MarkdownDataTracer.h"
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Logging/LogMacros.h"
#include "Trace/Utils/MarkdownFormattingUtils.h" // For spans
#include "Trace/Utils/MarkdownTracerUtils.h" // For name extraction
#include "K2Node_FunctionEntry.h" // Need FunctionEntry to find corresponding pin
#include "K2Node_FunctionResult.h" // Include self for casting checks if needed later
#include "EdGraph/EdGraph.h" // To get owning graph
#include "Logging/BP2AILog.h"


FString FNodeTraceHandlers_FunctionResult::HandleFunctionResult(
	TSharedPtr<const FBlueprintNode> Node,      // The FunctionResult node
	TSharedPtr<const FBlueprintPin> OutputPin, // The specific output pin on the Result node being traced
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
	check(Node.IsValid() && Node->NodeType == TEXT("FunctionResult"));
	check(Tracer);
	check(OutputPin.IsValid());
	UE_LOG(LogDataTracer, Verbose, TEXT("  HandleFunctionResult: Processing node %s for output pin %s"), *Node->Guid, *OutputPin->Name);

	// Find the corresponding INPUT pin on this SAME FunctionResult node
	// (Input pins on the Result node receive data from within the function)
	TSharedPtr<const FBlueprintPin> CorrespondingInputPin = Node->GetPin(OutputPin->Name, TEXT("EGPD_Input")); // Look for input pin with same name

	if (!CorrespondingInputPin.IsValid())
	{
		UE_LOG(LogDataTracer, Warning, TEXT("  HandleFunctionResult: Could not find corresponding INPUT pin for output pin '%s' on node %s"), *OutputPin->Name, *Node->Guid);
		return FMarkdownSpan::Error(FString::Printf(TEXT("[Result Input Pin '%s' Missing]"), *OutputPin->Name));
	}

	// Trace the value connected to this INPUT pin *within the current function graph*
	UE_LOG(LogDataTracer, Verbose, TEXT("  HandleFunctionResult: Tracing corresponding input pin '%s' (%s) recursively."), *CorrespondingInputPin->Name, *CorrespondingInputPin->Id);
	// Use the current 'CurrentNodesMap' map (which is the function's graph context)
	// Increment depth as we trace backwards from the input pin
	return Tracer->ResolvePinValueRecursive(CorrespondingInputPin, CurrentNodesMap, Depth + 1, VisitedPins);
}