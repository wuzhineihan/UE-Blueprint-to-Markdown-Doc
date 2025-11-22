/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Arrays.h


#pragma once

#include "CoreMinimal.h"
#include "Extractors/BlueprintDataExtractor.h"
#include "Trace/MarkdownDataTracer.h"


// Forward declarations
class FBlueprintNode;
class FBlueprintPin;
class FMarkdownDataTracer;

/**
 * Handlers for array operation node types.
 */
struct BP2AI_API FNodeTraceHandlers_Arrays
{
    /** Handles K2Node_MakeArray */
    static FString HandleMakeArray(
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

    /** Handles K2Node_GetArrayItem */
    static FString HandleGetArrayItem(
        TSharedPtr<const FBlueprintNode> Node,
        TSharedPtr<const FBlueprintPin> OutputPin, // The item output pin
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

    // --- ADDED DECLARATION ---
    /** Handles K2Node_CallArrayFunction (general handler) */
    static FString HandleCallArrayFunction(
        TSharedPtr<const FBlueprintNode> Node,
        TSharedPtr<const FBlueprintPin> OutputPin, // The specific output pin being traced
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
    // --- END ADDED DECLARATION ---


    /** Handles K2Node_CallArrayFunction (specifically for "Get" which is pure) */
    static FString HandleArrayGet(
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

    /** Handles K2Node_CallArrayFunction (specifically for "Length" which is pure) */
    static FString HandleArrayLength(
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