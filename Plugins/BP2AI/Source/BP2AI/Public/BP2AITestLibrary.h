/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BP2AITestLibrary.generated.h"

/**
 * Blueprint Function Library for BP2AI Testing
 * 提供可以在编辑器中直接调用的测试函数
 */
UCLASS()
class BP2AI_API UBP2AITestLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * 测试导出指定路径的蓝图
     * @param BlueprintPath 蓝图资产路径，如 "/Game/Test/BP_TestExport"
     */
    UFUNCTION(BlueprintCallable, Category = "BP2AI|Testing", meta = (CallInEditor = "true"))
    static bool TestExportBlueprintByPath(const FString& BlueprintPath);
};

