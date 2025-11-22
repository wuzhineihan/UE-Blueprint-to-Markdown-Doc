/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Sets.h


#pragma once

#include "CoreMinimal.h"
#include "Trace/MarkdownDataTracer.h" // For FNodeTraceHandlerFunc and other necessities

// Forward declarations (if not fully included by MarkdownDataTracer.h)
class FBlueprintNode;
class FBlueprintPin;
class FMarkdownDataTracer;
class FBlueprintDataExtractor;

struct BP2AI_API FNodeTraceHandlers_Sets
{
    /** Handles K2Node_MakeSet */
    static FString HandleMakeSet(
        TSharedPtr<const FBlueprintNode> Node,
        TSharedPtr<const FBlueprintPin> OutputPin, // The set output pin
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

    // Add other set-related handlers here in the future if needed
};