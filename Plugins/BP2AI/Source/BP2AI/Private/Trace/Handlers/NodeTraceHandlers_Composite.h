/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Composite.h
#pragma once

#include "CoreMinimal.h"
#include "Trace/MarkdownDataTracer.h" // For FNodeTraceHandlerFunc

// Forward declarations
class FBlueprintNode;
class FBlueprintPin;
class FMarkdownDataTracer;
class FBlueprintDataExtractor;

struct BP2AI_API FNodeTraceHandlers_Composite
{
    static FString HandleCompositeOutputPinValue(
        TSharedPtr<const FBlueprintNode> Node, // This is the Composite node itself
        TSharedPtr<const FBlueprintPin> OutputPinFromComposite, // The specific output pin of the Composite node being traced
        FMarkdownDataTracer* Tracer,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& OuterGraphNodesMap, // Nodes of the graph *calling* the composite
        int32 Depth,
        TSet<FString>& VisitedPins, // Visited pins in the *outer graph* context for this trace
        const FBlueprintDataExtractor& DataExtractor,
        TSharedPtr<const FBlueprintNode> CallingNodeForOuterArgs, // Effectively 'Node' itself
        const TMap<FString, TSharedPtr<FBlueprintNode>>* OuterNodesMapForOuterArgs, // Effectively '&OuterGraphNodesMap'
        bool bSymbolicTrace, // Overall symbolic preference for this trace
        const FString& OuterGraphContext
    );
};