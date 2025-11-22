/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/Handlers/NodeTraceHandlers_ObjectMgmt.h


#pragma once

#include "CoreMinimal.h"
#include "Extractors/BlueprintDataExtractor.h"
#include "Trace/MarkdownDataTracer.h"

// Forward declarations
class FBlueprintNode;
class FBlueprintPin;
class FMarkdownDataTracer;

/**
 * Handlers for object management node types (Spawn, AddComponent, CreateWidget, Cast, GetDefaults, GetSubsystem).
 */
struct BP2AI_API FNodeTraceHandlers_ObjectMgmt
{
    /** Handles K2Node_SpawnActorFromClass */
    static FString HandleSpawnActor(
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
    /** Handles K2Node_AddComponent */
    static FString HandleAddComponent(
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
    /** Handles K2Node_CreateWidget */
    static FString HandleCreateWidget(
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
    /** Handles K2Node_DynamicCast */
    static FString HandleDynamicCast(
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
    /** Handles K2Node_GetClassDefaults */
    static FString HandleGetClassDefaults(
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
    /** Handles K2Node_GetSubsystem and derived */
    static FString HandleGetSubsystem(
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

    static FString HandleIsValid(
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

    static FString HandleCreateObject(
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