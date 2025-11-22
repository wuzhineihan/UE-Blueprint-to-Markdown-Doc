/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

// Source/BP2AI/Private/Test/CurrentPhaseTest.cpp
#include "Test/CurrentPhaseTest.h"

#if !UE_BUILD_SHIPPING
#include "Exporters/BP2AIBatchExporter.h"
#include "Logging/BP2AILog.h"
#include "Engine/Blueprint.h"

namespace BP2AITests
{
    static const FString TestBlueprintPath = TEXT("/Game/Test/BP_TestExport");

    void ExecuteCurrentPhaseTest()
    {
        UE_LOG(LogBP2AI, Log, TEXT("[CurrentPhaseTest] Loading: %s"), *TestBlueprintPath);
        UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *TestBlueprintPath);
        if (!Blueprint)
        {
            UE_LOG(LogBP2AI, Error, TEXT("[CurrentPhaseTest] Failed to load blueprint"));
            return;
        }

        UE_LOG(LogBP2AI, Log, TEXT("[CurrentPhaseTest] Running complete blueprint export..."));
        FCompleteBlueprintData CompleteData = FBP2AIBatchExporter::ExportCompleteBlueprint(Blueprint, true);
        UE_LOG(LogBP2AI, Log, TEXT("[CurrentPhaseTest] Exported complete blueprint: %d graphs"), CompleteData.Graphs.Num());

        if (CompleteData.Graphs.Num() > 0)
        {
            UE_LOG(LogBP2AI, Log, TEXT("[CurrentPhaseTest] First Graph Markdown Preview:"));
            UE_LOG(LogBP2AI, Log, TEXT("--------------------------------------------------"));
            UE_LOG(LogBP2AI, Log, TEXT("%s"), *CompleteData.Graphs[0].Markdown);
            UE_LOG(LogBP2AI, Log, TEXT("--------------------------------------------------"));
        }
    }
}
#endif
