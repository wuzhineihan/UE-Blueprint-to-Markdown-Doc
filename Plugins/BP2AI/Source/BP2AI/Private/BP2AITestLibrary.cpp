/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

#include "BP2AITestLibrary.h"
#include "Logging/BP2AILog.h"

#if !UE_BUILD_SHIPPING
#include "Test/CurrentPhaseTest.h"
#include "Exporters/BP2AIBatchExporter.h"
#endif

void UBP2AITestLibrary::RunTask13Test()
{
    // Routine test start -> Log (not Warning)
    UE_LOG(LogBP2AI, Log, TEXT("🎮 BP2AI Test Library - RunTask13Test() CALLED"));
#if UE_BUILD_SHIPPING
    UE_LOG(LogBP2AI, Warning, TEXT("❌ Tests are DISABLED - UE_BUILD_SHIPPING is defined"));
    UE_LOG(LogBP2AI, Warning, TEXT("   Current Build Configuration: SHIPPING"));
#else
    UE_LOG(LogBP2AI, Log, TEXT("========================================"));
    UE_LOG(LogBP2AI, Log, TEXT("🎮 BP2AI Test Library - Running Task 1.3 Test"));
    UE_LOG(LogBP2AI, Log, TEXT("   Build Config: NOT SHIPPING (Test code enabled)"));
    UE_LOG(LogBP2AI, Log, TEXT("========================================"));
    BP2AITests::ExecuteCurrentPhaseTest();
    UE_LOG(LogBP2AI, Log, TEXT("========================================"));
    UE_LOG(LogBP2AI, Log, TEXT("✅ Test execution completed. Check Output Log for results."));
    UE_LOG(LogBP2AI, Log, TEXT("========================================"));
#endif
}

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

    // 可选：写出 index.json（调试/QA用，默认关闭）
    // FBP2AIBatchExporter::WriteIndexJson(Blueprint->GetName(), CompleteData.Graphs);

    if (!CompleteData.bIsInterface && CompleteData.Graphs.Num() == 0)
    {
        UE_LOG(LogBP2AI, Warning, TEXT("⚠️ TEST WARNING: No graphs exported (blueprint might be empty)"));
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

    UE_LOG(LogBP2AI, Log, TEXT("✅ TEST PASSED: Successfully exported complete blueprint document"));
    return true;
#endif
}
