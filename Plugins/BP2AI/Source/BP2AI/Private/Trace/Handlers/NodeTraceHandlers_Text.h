/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Text.h
#pragma once

#include "CoreMinimal.h"
#include "Extractors/BlueprintDataExtractor.h"
#include "Trace/MarkdownDataTracer.h"

// Forward declarations
class FBlueprintNode;
class FBlueprintPin;
class FMarkdownDataTracer;

/**
 * Handler for K2Node_FormatText nodes.
 */
struct BP2AI_API FNodeTraceHandlers_Text
{
	/** Handles tracing the output pin of K2Node_FormatText. */
	static FString HandleFormatText(
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
	);

	/** Handles K2Node_TextLiteral. */
	static FString HandleTextLiteral(
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