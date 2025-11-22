/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/ExecutionFlow/FlowHelpers/GraphDiscoveryHelper.h

#pragma once

#include "CoreMinimal.h"
#include "Trace/FMarkdownPathTracer.h" // For EUserGraphType

// Forward declarations
class FBlueprintDataExtractor;
class FMarkdownPathTracer;
class FMarkdownDataTracer;
class FBlueprintNode;
struct FTracingResults;
struct FGenerationSettings;
class FCategoryAnalysisHelper;

/**
 * Helper class for discovering all types of callable user graphs
 * Handles detection and initial processing of Functions, Macros, Custom Events, etc.
 */
class BP2AI_API FGraphDiscoveryHelper
{
public:
    FGraphDiscoveryHelper();
    ~FGraphDiscoveryHelper();

    /**
     * Discover pure user graphs from initially selected nodes
     */
    void PrescanForPureUserGraphs(
        const TMap<FString, TSharedPtr<FBlueprintNode>>& InSelectedNodesMap,
        FMarkdownPathTracer& InPathTracer,
        TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>& OutGraphsToDefineSeparately,
        TSet<FString>& InOutProcessedSeparateGraphPaths
    );

    /**
     * Execute discovery phase for a batch of graphs
     */
    TMap<FString, TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>> 
    ExecuteDiscoveryPhase(
        FMarkdownPathTracer& InPathTracer,
        FMarkdownDataTracer& InDataTracer,
        FBlueprintDataExtractor& InDataExtractor,
        TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>& GraphsToDiscoverThisPass,
        TSet<FString>& InOutProcessedSeparateGraphPaths,
        const FGenerationSettings& InSettings,
        FCategoryAnalysisHelper& CategoryHelper
    );
    /**
    * Process a single graph for discovery and categorization
    */
    void ProcessSingleGraphForDiscovery(
        FBlueprintDataExtractor& InDataExtractor,
        const TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>& GraphInfo,
        TMap<FString, TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>>& OutCategorizedGraphs,
        TSet<FString>& InOutProcessedSeparateGraphPaths,
        bool& OutHadNewGraphsAdded,
        FCategoryAnalysisHelper& CategoryHelper
    );
private:
   
};