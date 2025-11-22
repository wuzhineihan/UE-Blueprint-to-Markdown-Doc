/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Trace/ExecutionFlow/FlowHelpers/GraphDiscoveryHelper.cpp

#include "GraphDiscoveryHelper.h"

#include "CoreMinimal.h"
#include "CategoryAnalysisHelper.h" // For categorization functionality
#include "Trace/FMarkdownPathTracer.h"
#include "Trace/MarkdownDataTracer.h"
#include "Extractors/BlueprintDataExtractor.h"
#include "Models/BlueprintNode.h"
#include "Models/BlueprintPin.h"
#include "Trace/Generation/GenerationShared.h"
#include "Logging/BP2AILog.h"
#include "Trace/Utils/MarkdownTracerUtils.h"

FGraphDiscoveryHelper::FGraphDiscoveryHelper()
{
    UE_LOG(LogBP2AI, Log, TEXT("FGraphDiscoveryHelper: Initialized"));
}

FGraphDiscoveryHelper::~FGraphDiscoveryHelper()
{
    UE_LOG(LogBP2AI, Log, TEXT("FGraphDiscoveryHelper: Destroyed"));
}

void FGraphDiscoveryHelper::PrescanForPureUserGraphs(
    const TMap<FString, TSharedPtr<FBlueprintNode>>& InSelectedNodesMap,
    FMarkdownPathTracer& InPathTracer,
    TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>& OutGraphsToDefineSeparately,
    TSet<FString>& InOutProcessedSeparateGraphPaths)
{
    UE_LOG(LogPathTracer, Log, TEXT("FGraphDiscoveryHelper: Pre-scanning %d SELECTED nodes for PURE user graphs..."), InSelectedNodesMap.Num());
    
    int32 InitialCount = OutGraphsToDefineSeparately.Num();
    
    for (const auto& Pair : InSelectedNodesMap) {
        const TSharedPtr<FBlueprintNode>& Node = Pair.Value;
        if (!Node.IsValid()) continue;
        
        FString GraphPath = TEXT(""), GraphNameHint = TEXT("");
        auto DetectedType = InPathTracer.IsInternalUserGraph(Node, GraphPath, GraphNameHint);
        
        if (DetectedType == FMarkdownPathTracer::EUserGraphType::Function || 
            DetectedType == FMarkdownPathTracer::EUserGraphType::Macro) {
            
            bool bIsNodePure = false;
            FString PurityReason = TEXT("");
            
            if (Node->NodeType == TEXT("CallFunction")) {
                const FString* PurityProp = Node->RawProperties.Find(TEXT("bIsPureFunc"));
                bIsNodePure = PurityProp && PurityProp->ToBool();
                PurityReason = FString::Printf(TEXT("CallFunction(bIsPureFunc=%s)"), bIsNodePure ? TEXT("true") : TEXT("false"));
            }
            else if (Node->NodeType == TEXT("MacroInstance")) {
                // For MacroInstance, IsPure() method on FBlueprintNode model should be reliable
                // if it correctly reflects the macro's purity (e.g. by checking its template).
                bIsNodePure = Node->IsPure(); 
                PurityReason = FString::Printf(TEXT("MacroInstance(IsPure=%s)"), bIsNodePure ? TEXT("true") : TEXT("false"));
            }
            
            UE_LOG(LogPathTracer, Log, TEXT("  PRESCAN: Node '%s' (%s) - Type: %d, Pure: %s, Reason: %s"), 
                *Node->Name, *Node->Guid.Left(8), static_cast<int32>(DetectedType), 
                bIsNodePure ? TEXT("YES") : TEXT("NO"), *PurityReason);
            
            if (bIsNodePure) {
                // Check if this graph path (not the _DEF key yet) is something we might want to define.
                // The _DEF key is used to track if a definition has been *printed*.
                // Here, we are just collecting candidates.
                // AddUnique ensures we don't add the same graph multiple times from the prescan.
                bool bAlreadyQueued = false;
                for(const auto& ExistingGraph : OutGraphsToDefineSeparately)
                {
                    if(ExistingGraph.Get<1>() == GraphPath)
                    {
                        bAlreadyQueued = true;
                        break;
                    }
                }

                if (!bAlreadyQueued) {
                     // Check against InOutProcessedSeparateGraphPaths using just GraphPath to avoid re-adding if already processed in a broader sense
                    if (!InOutProcessedSeparateGraphPaths.Contains(GraphPath)) {
                        OutGraphsToDefineSeparately.Add(MakeTuple(GraphNameHint, GraphPath, DetectedType));
                         UE_LOG(LogPathTracer, Warning, TEXT("  >>> PRESCAN QUEUED: PURE user graph: %s (Path: %s, Type: %d)"), 
                            *GraphNameHint, *GraphPath, static_cast<int32>(DetectedType));
                    } else {
                         UE_LOG(LogPathTracer, Log, TEXT("  >>> PRESCAN SKIPPED (already in InOutProcessedSeparateGraphPaths): %s"), *GraphNameHint);
                    }
                } else {
                    UE_LOG(LogPathTracer, Log, TEXT("  >>> PRESCAN SKIPPED (already queued): %s"), *GraphNameHint);
                }
            }
        }
    }
    
    int32 AddedCount = OutGraphsToDefineSeparately.Num() - InitialCount;
    UE_LOG(LogPathTracer, Warning, TEXT("FGraphDiscoveryHelper: Pre-scan complete. Added %d pure graphs (Total in queue: %d)"), 
        AddedCount, OutGraphsToDefineSeparately.Num());
}

TMap<FString, TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>> 
FGraphDiscoveryHelper::ExecuteDiscoveryPhase(
    FMarkdownPathTracer& InPathTracer, // Not used directly here, but kept for signature consistency if needed later
    FMarkdownDataTracer& InDataTracer, // Not used directly here
    FBlueprintDataExtractor& InDataExtractor,
    TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>& GraphsToDiscoverThisPass, // Changed name for clarity
    TSet<FString>& InOutProcessedSeparateGraphPaths, // Tracks paths for which definition has been ATTEMPTED or PRINTED (_DEF suffix)
    const FGenerationSettings& InSettings, // Not used directly here
    FCategoryAnalysisHelper& CategoryHelper) // Helper for categorization
{
    UE_LOG(LogPathTracer, Error, TEXT("ExecuteDiscoveryPhase: ENTER. GraphsToDiscoverThisPass.Num() = %d"), 
        GraphsToDiscoverThisPass.Num());
    
    TMap<FString, TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>> CategorizedGraphs;
    
    if (GraphsToDiscoverThisPass.IsEmpty()) {
        UE_LOG(LogPathTracer, Error, TEXT("ExecuteDiscoveryPhase: No graphs in GraphsToDiscoverThisPass. Returning empty results."));
        return CategorizedGraphs;
    }

    bool bDummyFlagForProcessSingle = false; // OutHadNewGraphsAdded is not used by current ProcessSingleGraphForDiscovery

    for (const auto& GraphInfoTuple : GraphsToDiscoverThisPass)
    {
        const FString& GraphPath = GraphInfoTuple.Get<1>();
        FString DefKey = GraphPath + TEXT("_DEF"); // Key to check if definition was already printed/processed

        if (InOutProcessedSeparateGraphPaths.Contains(DefKey)) {
            UE_LOG(LogPathTracer, Log, TEXT("  ExecuteDiscoveryPhase: SKIPPING Graph (already processed for DEF): Hint='%s', Path='%s'"),
                *GraphInfoTuple.Get<0>(), *GraphPath);
            continue; 
        }
        
        UE_LOG(LogPathTracer, Error, TEXT("  ExecuteDiscoveryPhase: Calling ProcessSingleGraphForDiscovery for Hint='%s', Path='%s'"), 
            *GraphInfoTuple.Get<0>(), *GraphPath);
        ProcessSingleGraphForDiscovery(InDataExtractor, GraphInfoTuple, CategorizedGraphs, 
            InOutProcessedSeparateGraphPaths, bDummyFlagForProcessSingle, CategoryHelper); // Pass CategoryHelper
    }

    UE_LOG(LogPathTracer, Error, TEXT("ExecuteDiscoveryPhase: EXIT. Found %d categories."), CategorizedGraphs.Num());
    return CategorizedGraphs;
}

void FGraphDiscoveryHelper::ProcessSingleGraphForDiscovery(
    FBlueprintDataExtractor& InDataExtractor,
    const TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>& GraphInfo,
    TMap<FString, TArray<TTuple<FString, FString, FMarkdownPathTracer::EUserGraphType>>>& OutCategorizedGraphs,
    TSet<FString>& InOutProcessedSeparateGraphPaths, // This tracks general processing (path itself)
    bool& OutHadNewGraphsAdded, // This parameter is not used in the current implementation
    FCategoryAnalysisHelper& CategoryHelper) // Helper for categorization
{
    const FString& GraphNameHint = GraphInfo.Get<0>();
    const FString& GraphPath = GraphInfo.Get<1>();
    FMarkdownPathTracer::EUserGraphType GraphType = GraphInfo.Get<2>();

    
    UE_LOG(LogPathTracer, Log, TEXT("  ProcessSingleGraphForDiscovery: Processing '%s' (Path: %s, Type: %d)"), 
        *GraphNameHint, *GraphPath, static_cast<int32>(GraphType));

    // Mark the base path as processed for discovery to avoid re-categorizing in the same discovery pass if it appears multiple times.
    // The _DEF suffix in InOutProcessedSeparateGraphPaths is used by ExecutePrintingPhase to mark that a definition has been generated.
    // It's okay to add the base path here; ExecutePrintingPhase will check for _DEF.
    // InOutProcessedSeparateGraphPaths.Add(GraphPath); // This was in original prompt, but might be too aggressive if it prevents future definition if discovery fails once.
                                                 // Let's rely on _DEF key for printing. Discovery should be able to retry.

    TMap<FString, TSharedPtr<FBlueprintNode>> GraphNodes;
    if (!InDataExtractor.ExtractNodesFromGraph(GraphPath, GraphNodes) || GraphNodes.IsEmpty())
    {
        UE_LOG(LogPathTracer, Warning, TEXT("    Could not extract nodes for graph '%s' (Path: %s). Categorizing as Unknown."), *GraphNameHint, *GraphPath);
        OutCategorizedGraphs.FindOrAdd(TEXT("Unknown")).AddUnique(GraphInfo); // AddUnique to avoid duplicates in category list
        return;
    }

    FString CategoryKey = CategoryHelper.CategorizeGraphByType(GraphInfo, GraphNodes);
    OutCategorizedGraphs.FindOrAdd(CategoryKey).AddUnique(GraphInfo); // AddUnique
    UE_LOG(LogPathTracer, Log, TEXT("    Categorized '%s' as '%s'"), *GraphNameHint, *CategoryKey);
    OutHadNewGraphsAdded = false; // Explicitly set as it's an out param, though not used by caller in current flow.
}
