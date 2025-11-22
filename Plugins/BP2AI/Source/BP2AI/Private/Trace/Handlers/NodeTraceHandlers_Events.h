/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Events.h

#pragma once

#include "CoreMinimal.h"
#include "Extractors/BlueprintDataExtractor.h" // For FBlueprintDataExtractor type
#include "Trace/MarkdownDataTracer.h"       // For FMarkdownDataTracer type

// Forward declarations
class FBlueprintNode;
class FBlueprintPin;
// FMarkdownDataTracer is included, no need to forward declare if TFunction uses it directly
// FBlueprintDataExtractor is included

/**
 * Handlers for event node types (specifically for tracing output parameters).
 */
struct BP2AI_API FNodeTraceHandlers_Events
{
	/**
	 * Handles tracing output data pins from Event nodes
	 * (e.g., K2Node_Event, K2Node_CustomEvent, K2Node_InputAction...).
	 * This is the function registered in FMarkdownDataTracer.
	 */
	static FString HandleEventOutputParam(
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
	);
};
