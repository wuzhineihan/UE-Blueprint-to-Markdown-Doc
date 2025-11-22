/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/ExecutionFlow/FlowHelpers/DefinitionGenerationHelper.h

#pragma once

#include "CoreMinimal.h"
#include "Trace/Generation/GenerationShared.h" // For FGraphDefinitionEntry
#include "Trace/FMarkdownPathTracer.h" // For EUserGraphType
#include "Engine/Blueprint.h"


// Forward declarations
class FMarkdownPathTracer;
class FMarkdownDataTracer;
class FBlueprintDataExtractor;
class FBlueprintNode;
struct FGenerationSettings;

/**
 * Helper class for creating graph definitions for all categories
 * Handles definition creation for Functions, Macros, Custom Events, Interfaces, etc.
 */
class BP2AI_API FDefinitionGenerationHelper
{
public:
    FDefinitionGenerationHelper();
    ~FDefinitionGenerationHelper();

    /**
     * Create a complete graph definition entry
     */
    FGraphDefinitionEntry CreateGraphDefinition(
        FMarkdownPathTracer& InPathTracer,
        FMarkdownDataTracer& InDataTracer,
        FBlueprintDataExtractor& InDataExtractor,
        const TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>& GraphInfo,
        const FString& CategoryKey,
        TSet<FString>& InOutProcessedSeparateGraphPaths,
        const FGenerationSettings& InSettings,
        TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>& OutGraphsToDefineSeparately,
        TSet<FString>& InOutProcessedSeparateGraphPathsForDiscovery,
        const FString& RootBlueprintNameForTrace
    );

private:
    // Definition creation methods will be moved here
    void CollectInputSpecs(TSharedPtr<const FBlueprintNode> EntryNode, TArray<FString>& OutInputSpecs);
    
    void CollectOutputSpecs(
        TSharedPtr<const FBlueprintNode> ExitNode,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& GraphNodes,
        FMarkdownPathTracer::EUserGraphType GraphType,
        const FString& GraphNameHint,
        const FString& DefiningGraphAssetContext,
        FMarkdownDataTracer& InDataTracer,
        TArray<FString>& OutOutputSpecs
    );

    void CollectExecutionFlow(
        FMarkdownPathTracer& InPathTracer,
        FMarkdownDataTracer& InDataTracer,
        TSharedPtr<const FBlueprintNode> EntryNode,
        const TMap<FString, TSharedPtr<FBlueprintNode>>& GraphNodes,
        const FString& AssetContext,
        const FGenerationSettings& InSettings,
        TArray<FString>& OutExecutionFlow,
        TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>& OutGraphsToDefineSeparately,
        TSet<FString>& InOutProcessedSeparateGraphPaths
    );

    // Interface-specific methods
    void ExtractInterfaceSignature(
        const FString& GraphNameHint,
        TArray<FString>& OutInputSpecs,
        TArray<FString>& OutOutputSpecs
    );

    FString GetPropertyTypeDescription(FProperty* Property) const;

    // Node finding methods
    TSharedPtr<const FBlueprintNode> FindEntryNodeForInputs(
        const TMap<FString, TSharedPtr<FBlueprintNode>>& GraphNodes,
        FMarkdownPathTracer::EUserGraphType GraphType,
        const FString& SimpleNameFromHint
    );

    TSharedPtr<const FBlueprintNode> FindExitNodeForOutputs(
        const TMap<FString, TSharedPtr<FBlueprintNode>>& GraphNodes,
        FMarkdownPathTracer::EUserGraphType GraphType,
        const FString& SimpleNameFromHint
    );

    // Utility methods
    FString ExtractSimpleNameFromHint(const FString& GraphNameHint) const;
    FString DetermineAssetContext(const FString& GraphNameHint, const FString& RootBlueprintNameForTrace) const;
};