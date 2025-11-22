/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_Structs.h


#pragma once

#include "CoreMinimal.h"
#include "Extractors/BlueprintDataExtractor.h"
#include "Trace/MarkdownDataTracer.h"

// Forward declarations
class FBlueprintNode;
class FBlueprintPin;
class FMarkdownDataTracer;

/**
 * Handlers for struct operation node types.
 */
struct BP2AI_API FNodeTraceHandlers_Structs
{
    /** Handles K2Node_BreakStruct. */
    static FString HandleBreakStruct(
        TSharedPtr<const FBlueprintNode> Node,
        TSharedPtr<const FBlueprintPin> OutputPin, // The specific member pin being traced
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


    /** Handles K2Node_MakeStruct. */
    static FString HandleMakeStruct(
        TSharedPtr<const FBlueprintNode> Node,
        TSharedPtr<const FBlueprintPin> OutputPin, // The struct output pin
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

    /** Handles K2Node_SetFieldsInStruct. */
    static FString HandleSetFieldsInStruct(
        TSharedPtr<const FBlueprintNode> Node,
        TSharedPtr<const FBlueprintPin> OutputPin, // The struct output pin
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
};