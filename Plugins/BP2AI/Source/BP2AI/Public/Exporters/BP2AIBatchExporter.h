/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Public/Exporters/BP2AIBatchExporter.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/Blueprint.h"

class UEdGraph;
class UEdGraphNode;
struct FGenerationSettings;

// 阶段2 Task2.2: 增加结构化导出信息（统计 + Markdown）
struct FExportedGraphInfo
{
    FString GraphName;              // 图表名称
    FString Category;               // 类别: Event / Function / Macro / Delegate
    int32   NodeCount = 0;          // 节点数量
    int32   CharacterCount = 0;     // Markdown 字符数
    int32   LineCount = 0;          // Markdown 行数（按换行解析）
    int32   BlueprintBlockCount = 0;// ```blueprint 代码块数量（执行路径块数）
    FString Markdown;               // 完整 Markdown 文本
};

// 阶段4提前：蓝图级完整数据结构（GraphLogic + Metadata等）
struct FCompleteBlueprintData
{
    FString BlueprintName;          // 蓝图名称
    FString AssetPath;              // 资产路径
    bool bIsInterface = false;      // 是否为蓝图接口
    
    // 阶段2：GraphLogic（所有图表）
    TArray<FExportedGraphInfo> Graphs;
    
    // 阶段3：元数据
    struct FBlueprintMetadata
    {
        FString ClassName;
        FString ParentClass;
        FString AssetPath;
        TArray<FString> Interfaces;
    };

    struct FComponentInfo
    {
        FString Name;
        FString Type;
        FString ParentName; // 空字符串表示无父
    };

    struct FVariableInfo
    {
        FString Name;
        FString Type;
        FString DefaultValue;
        bool bIsPublic = false;
        FString Tooltip;
    };

    struct FFunctionParam
    {
        FString Name;
        FString Type;
        bool bIsReturn = false;
    };

    struct FFunctionInfo
    {
        FString Name;
        TArray<FFunctionParam> Parameters;
        FString ReturnType;
        bool bIsPure = false;
        bool bIsEvent = false;
    };

    struct FReferenceInfo
    {
        FString Path;
        FString Type;
        FString Source;
        bool bIsSoftReference = false;
    };

    FBlueprintMetadata Metadata;
    TArray<FComponentInfo> Components;
    TArray<FVariableInfo> Variables;
    TArray<FFunctionInfo> Functions;
    TArray<FReferenceInfo> References;

    // 渲染为单个蓝图级 Markdown 文档
    FString ToMarkdown() const;
};

/**
 * 批量导出器 - 自动化导出蓝图数据
 * 功能:
 * - 程序化导出单个图表的逻辑
 * - 导出整个蓝图的所有图表
 * - 不依赖 UI 或用户交互
 */
class BP2AI_API FBP2AIBatchExporter
{
public:
    FBP2AIBatchExporter();
    ~FBP2AIBatchExporter();

    // 单图表结构化导出（包含统计）
    FExportedGraphInfo ExportSingleGraphDetailed(UEdGraph* Graph, const FString& Category, bool bIncludeNestedFunctions = true);

    // 全蓝图结构化导出（返回所有图表数组）
    TArray<FExportedGraphInfo> ExportAllGraphsDetailed(UBlueprint* Blueprint, bool bIncludeNestedFunctions = true);

    // 蓝图级完整导出（阶段2 Task2.3）：聚合所有图表并返回完整数据
    static FCompleteBlueprintData ExportCompleteBlueprint(UBlueprint* Blueprint, bool bIncludeNestedFunctions = true);

    // 阶段3：导出元数据的静态函数
    static FCompleteBlueprintData::FBlueprintMetadata ExportMetadata(UBlueprint* Blueprint);
    static TArray<FCompleteBlueprintData::FComponentInfo> ExportComponents(UBlueprint* Blueprint);
    static TArray<FCompleteBlueprintData::FVariableInfo> ExportVariables(UBlueprint* Blueprint);
    static TArray<FCompleteBlueprintData::FFunctionInfo> ExportFunctions(UBlueprint* Blueprint);
    static TArray<FCompleteBlueprintData::FReferenceInfo> ExportReferences(UBlueprint* Blueprint);

    // 写出 Markdown 文档的工具方法（由调用方控制输出路径）
    static bool WriteCompleteBlueprintMarkdown(const FCompleteBlueprintData& Data, const FString& TargetFilePath, bool bCreateDirectories = true);

private:
    FGenerationSettings CreateDefaultSettings() const;
    bool IsGraphValid(UEdGraph* Graph) const;
    void LogExportResult(const FString& GraphName, const FString& Content) const;

    // 统计辅助：计算行数
    int32 CountLines(const FString& Content) const;
    // 统计辅助：扫描 ```blueprint 代码块数量
    int32 CountBlueprintBlocks(const FString& Content) const;

    // 后处理 Markdown：去除冗余块并添加 Target 信息
    FString PostProcessMarkdown(const FString& Markdown, UEdGraph* Graph) const;
};
