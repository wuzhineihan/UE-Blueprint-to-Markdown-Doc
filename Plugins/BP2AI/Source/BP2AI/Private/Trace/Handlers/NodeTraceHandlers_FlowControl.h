/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_FlowControl.h


#pragma once

#include "CoreMinimal.h"
#include "Extractors/BlueprintDataExtractor.h"
#include "Trace/MarkdownDataTracer.h"

// Forward declarations
class FBlueprintNode;
class FBlueprintPin;
class FMarkdownDataTracer;

/**
 * Handlers for flow control nodes that produce data values (e.g., Select).
 */
struct BP2AI_API FNodeTraceHandlers_FlowControl
{
	/** Handles K2Node_Select */
	static FString HandleSelect(
		TSharedPtr<const FBlueprintNode> Node,
		TSharedPtr<const FBlueprintPin> OutputPin, // The ReturnValue pin
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

	// Handlers for Switch, IfThenElse, etc., typically don't produce traceable values
	// directly in this manner, they control execution flow handled by PathTracer.
};