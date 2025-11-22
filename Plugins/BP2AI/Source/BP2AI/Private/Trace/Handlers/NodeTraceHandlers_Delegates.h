/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Delegates.h

#pragma once

#include "CoreMinimal.h"
#include "Extractors/BlueprintDataExtractor.h"
#include "Trace/MarkdownDataTracer.h"

// Forward declarations
class FBlueprintNode;
class FBlueprintPin;
class FMarkdownDataTracer;

/**
 * Handlers for delegate related node types.
 */
struct BP2AI_API FNodeTraceHandlers_Delegates
{
	/** Handles K2Node_CreateDelegate */
	static FString HandleDelegate(
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
	static FString HandleCreateDelegate(
		   TSharedPtr<const FBlueprintNode> Node,
		   TSharedPtr<const FBlueprintPin> OutputPin, // The delegate output pin
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

	

	// Handlers for Add, Remove, Assign, CallDelegate primarily affect execution flow
	// and are less likely to be traced for a specific *value*, but could be added if needed.
};