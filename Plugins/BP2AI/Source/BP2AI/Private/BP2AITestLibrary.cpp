/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

#include "BP2AITestLibrary.h"
#include "Logging/BP2AILog.h"
#include "Exporters/BP2AIBatchExporter.h"
#include "Misc/Paths.h"

bool UBP2AITestLibrary::TestExportBlueprintByPath(const FString& BlueprintPath)
{
    UE_LOG(LogBP2AI, Log, TEXT("🎮 BP2AI Test Library - TestExportBlueprintByPath() CALLED"));
#if UE_BUILD_SHIPPING
    UE_LOG(LogBP2AI, Warning, TEXT("❌ Tests are DISABLED - UE_BUILD_SHIPPING is defined"));
    return false;
#else
    UE_LOG(LogBP2AI, Log, TEXT("========================================"));
    UE_LOG(LogBP2AI, Log, TEXT("🎮 BP2AI Test Library - Testing Blueprint Export (Detailed)"));
    UE_LOG(LogBP2AI, Log, TEXT("   Blueprint Path: %s"), *BlueprintPath);
    UE_LOG(LogBP2AI, Log, TEXT("========================================"));

    UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *BlueprintPath);
    if (!Blueprint)
    {
        UE_LOG(LogBP2AI, Error, TEXT("❌ TEST FAILED: Could not load blueprint at path: %s"), *BlueprintPath);
        return false;
    }

    UE_LOG(LogBP2AI, Log, TEXT("✅ Blueprint loaded successfully: %s"), *Blueprint->GetName());

    // 使用新的蓝图级完整导出接口（一个蓝图一个文档）
    FCompleteBlueprintData CompleteData = FBP2AIBatchExporter::ExportCompleteBlueprint(Blueprint, true);

    if (!CompleteData.bIsInterface && CompleteData.Graphs.Num() == 0)
    {
        UE_LOG(LogBP2AI, Warning, TEXT("⚠️ TEST WARNING: No graphs exported (blueprint might be empty)"));
        return false;
    }

    const FString BaseDir = FPaths::ProjectSavedDir() / TEXT("BP2AI/Exports");
    const FString TargetFilePath = BaseDir / (CompleteData.BlueprintName + TEXT(".md"));
    const bool bSaved = FBP2AIBatchExporter::WriteCompleteBlueprintMarkdown(CompleteData, TargetFilePath, true);
    if (!bSaved)
    {
        UE_LOG(LogBP2AI, Warning, TEXT("⚠️ TEST FAILED: Could not save markdown to %s"), *TargetFilePath);
        return false;
    }

    // 简要汇总
    UE_LOG(LogBP2AI, Log, TEXT("📄 Blueprint Export Summary:"));
    UE_LOG(LogBP2AI, Log, TEXT("   Blueprint: %s"), *CompleteData.BlueprintName);
    UE_LOG(LogBP2AI, Log, TEXT("   Asset Path: %s"), *CompleteData.AssetPath);
    UE_LOG(LogBP2AI, Log, TEXT("   Total Graphs: %d"), CompleteData.Graphs.Num());
    
    int32 TotalNodes = 0;
    for (const FExportedGraphInfo& G : CompleteData.Graphs)
    {
        TotalNodes += G.NodeCount;
    }
    UE_LOG(LogBP2AI, Log, TEXT("   Total Nodes: %d"), TotalNodes);
    
    UE_LOG(LogBP2AI, Log, TEXT("   Graph List:"));
    for (const FExportedGraphInfo& G : CompleteData.Graphs)
    {
        UE_LOG(LogBP2AI, Log, TEXT("      [%s] %s | Nodes=%d"), *G.Category, *G.GraphName, G.NodeCount);
    }

    UE_LOG(LogBP2AI, Log, TEXT("✅ TEST PASSED: Successfully exported complete blueprint document -> %s"), *TargetFilePath);
    return true;
#endif
}
