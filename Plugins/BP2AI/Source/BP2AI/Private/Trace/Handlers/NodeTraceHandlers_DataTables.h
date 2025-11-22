/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_DataTables.h
#pragma once

#include "CoreMinimal.h"
#include "Trace/MarkdownDataTracer.h" // For FNodeTraceHandlerFunc and FMarkdownDataTracer
#include "Trace/Handlers/NodeTraceHandlers_DataTables.h"

// Forward declarations
class FBlueprintNode;
class FBlueprintPin;
class FBlueprintDataExtractor; // Though not directly used by this specific handler, good for consistency

/**
 * Handlers for data-related node types (e.g., Data Tables).
 */
struct BP2AI_API FNodeTraceHandlers_DataTables
{
	/** Handles K2Node_GetDataTableRow */
	static FString HandleGetDataTableRow(
		TSharedPtr<const FBlueprintNode> Node,      // The GetDataTableRow node
		TSharedPtr<const FBlueprintPin> OutputPin, // The specific pin on this node whose value we want
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

	// Add other data-related handlers here in the future (e.g., CurveTable)
};