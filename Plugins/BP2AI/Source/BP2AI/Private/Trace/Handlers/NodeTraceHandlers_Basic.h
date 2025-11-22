/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Basic.h


#pragma once

#include "CoreMinimal.h"
#include "Extractors/BlueprintDataExtractor.h"
#include "Trace/MarkdownDataTracer.h" // Include for FNodeTraceHandlerFunc type alias and FMarkdownSpan

// Forward declarations
class FBlueprintNode;
class FBlueprintPin;
class FMarkdownDataTracer;

/**
 * Handlers for basic Blueprint node types.
 */
struct BP2AI_API FNodeTraceHandlers_Basic // Using API macro for potential future use, adjust if not needed
{
	/** Handles K2Node_VariableGet. */
	static FString HandleVariableGet(
	   TSharedPtr<const FBlueprintNode> Node,
	   TSharedPtr<const FBlueprintPin> OutputPin,
	   FMarkdownDataTracer* Tracer,
	   const TMap<FString, TSharedPtr<FBlueprintNode>>& AllNodes, // This is CurrentNodesMap
	   int32 Depth,
	   TSet<FString>& VisitedPins,
	   const FBlueprintDataExtractor& DataExtractor,
	   // --- ADDED MISSING PARAMETERS ---
	   TSharedPtr<const FBlueprintNode> CallingNode,
	   const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
	   bool bSymbolicTrace,
	   const FString& CurrentBlueprintContext
	   // --- END ADDED ---
   );

	/** Handles K2Node_Literal. */
	static FString HandleLiteral(
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

	/** Handles K2Node_Self. */
	static FString HandleSelf(
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

			/**
			 * Handles tracing the output pins of a FunctionEntry node.
			 * This handler checks the current call-site arguments context and returns
			 * the pre-resolved argument values when available.
			 */

		
	/** Handles K2Node_VariableSet (specifically for tracing the Output_Get passthrough). */
	static FString HandleVariableSet(
		TSharedPtr<const FBlueprintNode> Node,
		TSharedPtr<const FBlueprintPin> OutputPin, // The pin being traced (e.g., Output_Get)
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

	static FString HandleTunnel(
		TSharedPtr<const FBlueprintNode> Node,            // The Tunnel node itself
		TSharedPtr<const FBlueprintPin> PinBeingTraced, // The specific pin on the Tunnel node we are tracing
		FMarkdownDataTracer* Tracer,
		const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap, // The map for the *current* graph context (could be main graph or sub-graph)
		int32 Depth,
		TSet<FString>& VisitedPins,
		const FBlueprintDataExtractor& DataExtractor,
		TSharedPtr<const FBlueprintNode> CallingNode,
		const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap,
		bool bSymbolicTrace,
		const FString& CurrentBlueprintContext
	);

	// ADDED HANDLERS FOR KNOT AND COMMENT
	static FString HandleKnot(TSharedPtr<const FBlueprintNode> Node, TSharedPtr<const FBlueprintPin> PinBeingTraced, FMarkdownDataTracer* Tracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap, int32 Depth, TSet<FString>& VisitedPins, const FBlueprintDataExtractor& DataExtractor, TSharedPtr<const FBlueprintNode> CallingNode, const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap, bool bSymbolicTrace);
	static FString HandleComment(TSharedPtr<const FBlueprintNode> Node, TSharedPtr<const FBlueprintPin> PinBeingTraced, FMarkdownDataTracer* Tracer, const TMap<FString, TSharedPtr<FBlueprintNode>>& CurrentNodesMap, int32 Depth, TSet<FString>& VisitedPins, const FBlueprintDataExtractor& DataExtractor, TSharedPtr<const FBlueprintNode> CallingNode, const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMap, bool bSymbolicTrace);
};