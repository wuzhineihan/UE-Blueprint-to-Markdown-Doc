/*
 * Copyright (c) 2025 A-Maze Games
 * Website: www.a-maze.games
 * All rights reserved.
 */
// Source/BP2AI/Private/Exporters/BP2AIBatchExporter.cpp

#include "Exporters/BP2AIBatchExporter.h"
#include "Logging/BP2AILog.h"
#include "Settings/BP2AIExportConfig.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "Engine/Blueprint.h"
#include "Trace/ExecutionFlow/ExecutionFlowGenerator.h"
#include "Trace/MarkdownGenerationContext.h"
#include "Trace/Generation/GenerationShared.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "K2Node_CallFunction.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "K2Node_Event.h"
#include "K2Node_CustomEvent.h"
#include "UObject/UnrealType.h"

// 前置声明：蓝图级 Markdown 写出
static bool WriteBlueprintMarkdown(const FCompleteBlueprintData& Data);
static bool IsInterfaceGraph(UEdGraph* Graph, UBlueprint* Blueprint, const TSet<FName>& InterfaceFuncs);

static FString GetPropertyTypeString(FProperty* Property)
{
    if (!Property)
    {
        return TEXT("void");
    }

    FString ExtendedType; // captures template info (e.g., TArray)
    FString BaseType = Property->GetCPPType(&ExtendedType, 0);
    if (!ExtendedType.IsEmpty())
    {
        if (!BaseType.IsEmpty())
        {
            BaseType += TEXT(" ");
        }
        BaseType += ExtendedType;
    }

    if (BaseType.IsEmpty())
    {
        BaseType = TEXT("void");
    }

    return BaseType;
}

static TArray<FCompleteBlueprintData::FFunctionInfo> ExportInterfaceFunctionSignatures(UBlueprint* Blueprint)
{
    TArray<FCompleteBlueprintData::FFunctionInfo> Funcs;
    if (!Blueprint)
    {
        return Funcs;
    }

    UClass* InterfaceClass = Blueprint->SkeletonGeneratedClass;
    if (!InterfaceClass)
    {
        InterfaceClass = Blueprint->GeneratedClass;
    }

    if (!InterfaceClass)
    {
        UE_LOG(LogBP2AI, Warning, TEXT("ExportInterfaceFunctionSignatures: Missing Skeleton/Generated class for blueprint interface %s"), *Blueprint->GetName());
        return Funcs;
    }

    TSet<FName> ProcessedNames;
    for (TFieldIterator<UFunction> FuncIt(InterfaceClass, EFieldIteratorFlags::ExcludeSuper); FuncIt; ++FuncIt)
    {
        UFunction* Func = *FuncIt;
        if (!Func)
        {
            continue;
        }

        const FName FuncName = Func->GetFName();
        if (ProcessedNames.Contains(FuncName))
        {
            continue;
        }

        const FString FuncNameString = Func->GetName();
        if (FuncNameString.StartsWith(TEXT("ExecuteUbergraph")))
        {
            continue;
        }

        if (!Func->HasAnyFunctionFlags(FUNC_BlueprintCallable | FUNC_BlueprintEvent))
        {
            continue;
        }

        FCompleteBlueprintData::FFunctionInfo Info;
        Info.Name = FuncNameString;
        Info.bIsEvent = false;
        Info.bIsPure = Func->HasAnyFunctionFlags(FUNC_BlueprintPure);
        Info.ReturnType = TEXT("void");

        if (FProperty* ReturnProp = Func->GetReturnProperty())
        {
            Info.ReturnType = GetPropertyTypeString(ReturnProp);
        }

        for (TFieldIterator<FProperty> PropIt(Func); PropIt && (PropIt->PropertyFlags & CPF_Parm); ++PropIt)
        {
            FProperty* Prop = *PropIt;
            if (!Prop || Prop->HasAnyPropertyFlags(CPF_ReturnParm))
            {
                continue;
            }

            FCompleteBlueprintData::FFunctionParam Param;
            Param.Name = Prop->GetName();
            Param.Type = GetPropertyTypeString(Prop);
            Param.bIsReturn = false;
            Info.Parameters.Add(Param);
        }

        Funcs.Add(Info);
        ProcessedNames.Add(FuncName);
    }

    Funcs.Sort([](const FCompleteBlueprintData::FFunctionInfo& A, const FCompleteBlueprintData::FFunctionInfo& B)
    {
        return A.Name < B.Name;
    });

    return Funcs;
}

// 调试开关：是否为每个图表单独写 md 文件（默认关闭）
// 移除未使用变量以消除警告
// static bool GBP2AI_WritePerGraphMarkdown = false;

FBP2AIBatchExporter::FBP2AIBatchExporter()
{
    UE_LOG(LogBP2AI, Log, TEXT("BP2AIBatchExporter: Initialized"));
}

FBP2AIBatchExporter::~FBP2AIBatchExporter()
{
    UE_LOG(LogBP2AI, Log, TEXT("BP2AIBatchExporter: Destroyed"));
}

FGenerationSettings FBP2AIBatchExporter::CreateDefaultSettings() const
{
    FGenerationSettings Settings;
    
    // 从配置文件读取设置
    Settings.bTraceAllSelected = true;                                              // 追踪所有选中的执行节点
    Settings.bDefineUserGraphsSeparately = BP2AIExportConfig::bSeparateUserGraphs; // 从配置读取
    Settings.bExpandCompositesInline = false;                                       // 不内联展开
    Settings.bShowTrivialDefaultParams = BP2AIExportConfig::bShowDefaultParams;     // 从配置读取
    
    // 所有类别默认可见（构造函数已初始化，这里可以覆盖）
    Settings.CategoryVisibility.Add(EDocumentationGraphCategory::Functions, true);
    Settings.CategoryVisibility.Add(EDocumentationGraphCategory::CustomEvents, true);
    Settings.CategoryVisibility.Add(EDocumentationGraphCategory::CollapsedGraphs, true);
    Settings.CategoryVisibility.Add(EDocumentationGraphCategory::ExecutableMacros, true);
    Settings.CategoryVisibility.Add(EDocumentationGraphCategory::PureMacros, true);
    Settings.CategoryVisibility.Add(EDocumentationGraphCategory::Interfaces, true);
    Settings.CategoryVisibility.Add(EDocumentationGraphCategory::PureFunctions, true);
    
    return Settings;
}

bool FBP2AIBatchExporter::IsGraphValid(UEdGraph* Graph) const
{
    if (!Graph)
    {
        UE_LOG(LogBP2AI, Error, TEXT("BP2AIBatchExporter: Graph is null"));
        return false;
    }
    
    if (Graph->Nodes.Num() == 0)
    {
        UE_LOG(LogBP2AI, Warning, TEXT("BP2AIBatchExporter: Graph '%s' has no nodes"), *Graph->GetName());
        return false;
    }
    
    return true;
}

void FBP2AIBatchExporter::LogExportResult(const FString& GraphName, const FString& Content) const
{
    const int32 CharCount = Content.Len();
    int32 LineCount = 0;
    if (!Content.IsEmpty())
    {
        LineCount = 1;
        for (TCHAR C : Content)
        {
            if (C == TEXT('\n')) { LineCount++; }
        }
    }
    UE_LOG(LogBP2AI, Log, TEXT("✅ Exported '%s': %d characters, %d lines"), *GraphName, CharCount, LineCount);

    // ExecFlow preview: 从配置文件读取是否启用预览
    if (!BP2AIExportConfig::bPreviewEnabled) { return; }

    if (Content.IsEmpty())
    {
        UE_LOG(LogBP2AI, Log, TEXT("--- ExecFlow Preview for '%s': <no content> ---"), *GraphName);
        return;
    }

    const int32 MaxSearchWindow = 50000; // Safety guard for huge documents
    const FString SearchSource = (Content.Len() > MaxSearchWindow) ? Content.Left(MaxSearchWindow) : Content;

    // Scan all ```blueprint blocks
    struct FBlock { int32 Start; int32 End; };
    TArray<FBlock> Blocks;
    int32 Seek = 0;
    while (true)
    {
        int32 FenceStart = SearchSource.Find(TEXT("```blueprint"), ESearchCase::IgnoreCase, ESearchDir::FromStart, Seek);
        if (FenceStart == INDEX_NONE) { break; }
        int32 BlockStart = SearchSource.Find(TEXT("\n"), ESearchCase::IgnoreCase, ESearchDir::FromStart, FenceStart);
        if (BlockStart == INDEX_NONE) { break; }
        int32 FenceEnd = SearchSource.Find(TEXT("```"), ESearchCase::IgnoreCase, ESearchDir::FromStart, BlockStart + 1);
        if (FenceEnd == INDEX_NONE) { break; }
        Blocks.Add({ BlockStart + 1, FenceEnd });
        Seek = FenceEnd + 3; // continue after closing fence
    }

    // Fallback: no fenced blocks, preview full content as one block
    if (Blocks.Num() == 0)
    {
        Blocks.Add({ 0, SearchSource.Len() });
    }

    // 从配置文件读取预览参数
    const int32 MaxBlocksToShow = BP2AIExportConfig::PreviewMaxBlocks;
    const int32 LinesPerBlock   = BP2AIExportConfig::PreviewLinesPerBlock;
    const int32 MaxLineLength   = BP2AIExportConfig::PreviewMaxLineLength;

    const int32 BlocksFound = Blocks.Num();
    const int32 BlocksToShow = FMath::Min(MaxBlocksToShow, BlocksFound);

    UE_LOG(LogBP2AI, Log, TEXT("----- ExecFlow Preview for Graph: %s (blocks: showing %d/%d) -----"), *GraphName, BlocksToShow, BlocksFound);

    for (int32 b = 0; b < BlocksToShow; ++b)
    {
        const FBlock& Blk = Blocks[b];
        const FString BlockText = SearchSource.Mid(Blk.Start, Blk.End - Blk.Start);
        TArray<FString> Lines; BlockText.ParseIntoArrayLines(Lines);
        const int32 Show = FMath::Min(LinesPerBlock, Lines.Num());

        // Try to extract a short title from the first non-empty line
        FString Title;
        for (const FString& L : Lines)
        {
            if (!L.TrimStartAndEnd().IsEmpty()) { Title = L.TrimStartAndEnd(); break; }
        }

        if (!Title.IsEmpty() && Title.Len() > 120)
        {
            Title.LeftInline(120, /*bAllowShrinking*/ false); Title += TEXT(" ...[truncated]");
        }

        UE_LOG(LogBP2AI, Log, TEXT("[Block %d/%d] %s"), b + 1, BlocksToShow, Title.IsEmpty() ? TEXT("<no title>") : *Title);
        for (int32 i = 0; i < Show; ++i)
        {
            FString Line = Lines[i];
            if (Line.Len() > MaxLineLength) { Line.LeftInline(MaxLineLength, false); Line += TEXT(" ...[truncated]"); }
            UE_LOG(LogBP2AI, Log, TEXT("   [%02d] %s"), i + 1, *Line);
        }
        if (Lines.Num() > Show)
        {
            UE_LOG(LogBP2AI, Log, TEXT("   ... (%d more line(s) in this block)"), Lines.Num() - Show);
        }
        if (b < BlocksToShow - 1)
        {
            UE_LOG(LogBP2AI, Log, TEXT("   ---"));
        }
    }

    if (BlocksFound > BlocksToShow)
    {
        UE_LOG(LogBP2AI, Log, TEXT("... ExecFlow preview truncated: %d more block(s) not shown. See Markdown export for full content."), BlocksFound - BlocksToShow);
    }

    UE_LOG(LogBP2AI, Log, TEXT("----- End ExecFlow Preview for Graph: %s -----"), *GraphName);
}

int32 FBP2AIBatchExporter::CountLines(const FString& Content) const
{
    if (Content.IsEmpty()) { return 0; }
    int32 Lines = 1; // 至少一行
    for (TCHAR C : Content) { if (C == TEXT('\n')) { Lines++; } }
    return Lines;
}

int32 FBP2AIBatchExporter::CountBlueprintBlocks(const FString& Content) const
{
    if (Content.IsEmpty()) { return 0; }
    int32 Count = 0;
    int32 Seek = 0;
    while (true)
    {
        int32 FenceStart = Content.Find(TEXT("```blueprint"), ESearchCase::IgnoreCase, ESearchDir::FromStart, Seek);
        if (FenceStart == INDEX_NONE) { break; }
        int32 BlockStart = Content.Find(TEXT("\n"), ESearchCase::IgnoreCase, ESearchDir::FromStart, FenceStart);
        if (BlockStart == INDEX_NONE) { break; }
        int32 FenceEnd = Content.Find(TEXT("```"), ESearchCase::IgnoreCase, ESearchDir::FromStart, BlockStart + 1);
        if (FenceEnd == INDEX_NONE) { break; }
        Count++;
        Seek = FenceEnd + 3;
    }
    return Count;
}

FString FBP2AIBatchExporter::PostProcessMarkdown(const FString& Markdown, UEdGraph* Graph) const
{
    if (Markdown.IsEmpty()) { return Markdown; }

    FString Result = Markdown;

    // 1. 去除冗余块 (Remove Redundant Blocks)
    int32 Seek = 0;
    while (true)
    {
        int32 BlockStart = Result.Find(TEXT("**Trace Start:"), ESearchCase::IgnoreCase, ESearchDir::FromStart, Seek);
        if (BlockStart == INDEX_NONE) { break; }

        // 查找块结束（下一个 Trace Start 或文件末尾）
        int32 NextBlockStart = Result.Find(TEXT("**Trace Start:"), ESearchCase::IgnoreCase, ESearchDir::FromStart, BlockStart + 14);
        int32 BlockEnd = (NextBlockStart == INDEX_NONE) ? Result.Len() : NextBlockStart;

        FString BlockContent = Result.Mid(BlockStart, BlockEnd - BlockStart);
        
        // Check if the blueprint code block inside this section is trivial/redundant
        bool bIsRedundant = false;
        int32 CodeStart = BlockContent.Find(TEXT("```blueprint"));
        if (CodeStart != INDEX_NONE)
        {
             int32 CodeEnd = BlockContent.Find(TEXT("```"), ESearchCase::IgnoreCase, ESearchDir::FromStart, CodeStart + 12);
             if (CodeEnd != INDEX_NONE)
             {
                 FString CodeBody = BlockContent.Mid(CodeStart + 12, CodeEnd - (CodeStart + 12));
                 CodeBody.TrimStartAndEndInline();
                 
                 // Only consider it redundant if it's a single line pointing to "Previously detailed"
                 if (CodeBody.Contains(TEXT("Previously detailed")) && !CodeBody.Contains(TEXT("\n")))
                 {
                     bIsRedundant = true;
                 }
             }
        }

        if (bIsRedundant)
        {
            // 移除该块
            Result.RemoveAt(BlockStart, BlockEnd - BlockStart);
            // 不更新 Seek，因为后面的内容前移了
            continue;
        }
        
        Seek = BlockStart + 14; // 继续搜索下一个
    }

    // 2. 添加 Target 信息 (Add Target Info)
    if (Graph)
    {
        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (UK2Node_CallFunction* CallFuncNode = Cast<UK2Node_CallFunction>(Node))
            {
                FString FuncName = CallFuncNode->GetFunctionName().ToString();
                FString TargetClassName = TEXT("Self");
                
                // 尝试从 Target 引脚获取类型
                UEdGraphPin* TargetPin = CallFuncNode->FindPin(TEXT("self"));
                if (!TargetPin) { TargetPin = CallFuncNode->FindPin(TEXT("Target")); }

                if (TargetPin && TargetPin->LinkedTo.Num() > 0)
                {
                    if (UEdGraphPin* LinkedPin = TargetPin->LinkedTo[0])
                    {
                        if (LinkedPin->PinType.PinSubCategoryObject.IsValid())
                        {
                            TargetClassName = LinkedPin->PinType.PinSubCategoryObject->GetName();
                        }
                        else if (LinkedPin->PinType.PinCategory == TEXT("object"))
                        {
                            TargetClassName = TEXT("Object");
                        }
                    }
                }
                else
                {
                    // 无连接，通常是 Self 或静态函数库
                    if (UFunction* TargetFunc = CallFuncNode->GetTargetFunction())
                    {
                        if (UClass* OwnerClass = TargetFunc->GetOwnerClass())
                        {
                            TargetClassName = OwnerClass->GetName();
                        }
                    }
                }

                // 在 Markdown 中查找并注入 Target 信息
                // 查找模式: "* FuncName("
                // 注入位置: 行尾
                int32 SearchPos = 0;
                FString SearchPattern = FString::Printf(TEXT("* %s("), *FuncName);
                
                // 防止死循环或过度搜索，限制次数（可选）
                while (true)
                {
                    int32 FoundPos = Result.Find(SearchPattern, ESearchCase::IgnoreCase, ESearchDir::FromStart, SearchPos);
                    if (FoundPos == INDEX_NONE) { break; }
                    
                    int32 LineEnd = Result.Find(TEXT("\n"), ESearchCase::IgnoreCase, ESearchDir::FromStart, FoundPos);
                    if (LineEnd == INDEX_NONE) { LineEnd = Result.Len(); }
                    
                    FString LineContent = Result.Mid(FoundPos, LineEnd - FoundPos);
                    // 避免重复添加
                    if (!LineContent.Contains(TEXT("(Target:")))
                    {
                        FString Insertion = FString::Printf(TEXT(" (Target: %s)"), *TargetClassName);
                        Result.InsertAt(LineEnd, Insertion);
                        LineEnd += Insertion.Len();
                    }
                    
                    SearchPos = LineEnd;
                }
            }
        }
    }

    return Result;
}

FExportedGraphInfo FBP2AIBatchExporter::ExportSingleGraphDetailed(UEdGraph* Graph, const FString& Category, bool bIncludeNestedFunctions)
{
    FExportedGraphInfo Info;
    if (!IsGraphValid(Graph)) { return Info; }
    Info.GraphName = Graph->GetName();
    Info.Category = Category;
    Info.NodeCount = Graph->Nodes.Num();

    TArray<UEdGraphNode*> AllNodes = Graph->Nodes;
    FGenerationSettings Settings = CreateDefaultSettings();
    Settings.bDefineUserGraphsSeparately = bIncludeNestedFunctions;
    FMarkdownGenerationContext Context(FMarkdownGenerationContext::EOutputFormat::RawMarkdown);
    FExecutionFlowGenerator Generator;
    Info.Markdown = Generator.GenerateDocumentForNodes(AllNodes, Settings, Context);

    // Task 2.5: Post-process Markdown (Remove redundant blocks & add Target info)
    Info.Markdown = PostProcessMarkdown(Info.Markdown, Graph);

    Info.CharacterCount = Info.Markdown.Len();
    Info.LineCount = CountLines(Info.Markdown);
    Info.BlueprintBlockCount = CountBlueprintBlocks(Info.Markdown);

    // 复用已有日志输出（保持兼容与可读性）
    LogExportResult(Info.GraphName, Info.Markdown);

    return Info;
}

static TSet<FName> GetInterfaceFunctionNames(UBlueprint* Blueprint)
{
    TSet<FName> Names;
    if (!Blueprint) return Names;

    for (const FBPInterfaceDescription& Iface : Blueprint->ImplementedInterfaces)
    {
        UClass* InterfaceClass = Iface.Interface.Get();
        if (InterfaceClass)
        {
            UE_LOG(LogBP2AI, Log, TEXT("Found Interface: %s"), *InterfaceClass->GetName());
            for (TFieldIterator<UFunction> Fit(InterfaceClass, EFieldIteratorFlags::IncludeSuper); Fit; ++Fit)
            {
                UFunction* Func = *Fit;
                if (Func && Func->HasAnyFunctionFlags(FUNC_BlueprintCallable | FUNC_BlueprintEvent))
                {
                    // Filter out internal generated functions
                    if (Func->GetName().StartsWith(TEXT("ExecuteUbergraph")))
                    {
                        continue;
                    }

                    Names.Add(Func->GetFName());
                    UE_LOG(LogBP2AI, Log, TEXT("   - Found Interface Function: %s"), *Func->GetName());
                }
            }
        }
        else
        {
             UE_LOG(LogBP2AI, Warning, TEXT("Found Interface but class is null"));
        }
    }
    return Names;
}

TArray<FExportedGraphInfo> FBP2AIBatchExporter::ExportAllGraphsDetailed(UBlueprint* Blueprint, bool bIncludeNestedFunctions)
{
    TArray<FExportedGraphInfo> Result;
    if (!Blueprint)
    {
        UE_LOG(LogBP2AI, Error, TEXT("ExportAllGraphsDetailed: Blueprint is null"));
        return Result;
    }
    UE_LOG(LogBP2AI, Log, TEXT("========================================"));
    UE_LOG(LogBP2AI, Log, TEXT("BP2AIBatchExporter: Detailed export for blueprint '%s'"), *Blueprint->GetName());
    UE_LOG(LogBP2AI, Log, TEXT("========================================"));

    auto ExportArray = [&](const TArray<UEdGraph*>& GraphArray, const TCHAR* CategoryLabel)
    {
        for (UEdGraph* G : GraphArray)
        {
            if (!G) { continue; }
            Result.Add(ExportSingleGraphDetailed(G, CategoryLabel, bIncludeNestedFunctions));
        }
    };

    // Get all interface function names
    TSet<FName> InterfaceFuncs = GetInterfaceFunctionNames(Blueprint);

    // Collect all function graphs, including interface implementation graphs
    TArray<UEdGraph*> AllFunctionGraphs = Blueprint->FunctionGraphs;
    TSet<UEdGraph*> SeenGraphs;
    for (UEdGraph* Existing : AllFunctionGraphs)
    {
        if (Existing)
        {
            SeenGraphs.Add(Existing);
        }
    }
    for (const FBPInterfaceDescription& Iface : Blueprint->ImplementedInterfaces)
    {
        for (UEdGraph* InterfaceGraph : Iface.Graphs)
        {
            if (InterfaceGraph && !SeenGraphs.Contains(InterfaceGraph))
            {
                SeenGraphs.Add(InterfaceGraph);
                AllFunctionGraphs.Add(InterfaceGraph);
            }
        }
    }

    // Split function graphs into Standard and Interface categories
    TArray<UEdGraph*> StandardFunctions;
    TArray<UEdGraph*> InterfaceFunctions;
    for (UEdGraph* G : AllFunctionGraphs)
    {
        if (G)
        {
            if (IsInterfaceGraph(G, Blueprint, InterfaceFuncs))
            {
                InterfaceFunctions.Add(G);
            }
            else
            {
                StandardFunctions.Add(G);
            }
        }
    }

    ExportArray(Blueprint->UbergraphPages, TEXT("Event"));
    ExportArray(StandardFunctions, TEXT("Function"));
    ExportArray(InterfaceFunctions, TEXT("Interface"));
    ExportArray(Blueprint->MacroGraphs, TEXT("Macro"));
    ExportArray(Blueprint->DelegateSignatureGraphs, TEXT("Delegate"));

    // 汇总统计
    int32 TotalChars = 0, TotalLines = 0, TotalBlocks = 0, TotalNodes = 0;
    for (const FExportedGraphInfo& G : Result)
    {
        TotalChars += G.CharacterCount;
        TotalLines += G.LineCount;
        TotalBlocks += G.BlueprintBlockCount;
        TotalNodes += G.NodeCount;
    }
    UE_LOG(LogBP2AI, Log, TEXT("📊 Detailed Export Summary:"));
    UE_LOG(LogBP2AI, Log, TEXT("   Graphs: %d"), Result.Num());
    UE_LOG(LogBP2AI, Log, TEXT("   Total Nodes: %d"), TotalNodes);
    UE_LOG(LogBP2AI, Log, TEXT("   Total Characters: %d"), TotalChars);
    UE_LOG(LogBP2AI, Log, TEXT("   Total Lines: %d"), TotalLines);
    UE_LOG(LogBP2AI, Log, TEXT("   Total Blueprint Blocks: %d"), TotalBlocks);

    UE_LOG(LogBP2AI, Log, TEXT("   Per-Graph Overview:"));
    for (const FExportedGraphInfo& G : Result)
    {
        UE_LOG(LogBP2AI, Log, TEXT("      [%s] %s | Nodes=%d, Chars=%d, Lines=%d, Blocks=%d"), *G.Category, *G.GraphName, G.NodeCount, G.CharacterCount, G.LineCount, G.BlueprintBlockCount);
    }
    UE_LOG(LogBP2AI, Log, TEXT("========================================"));

    return Result;
}

FCompleteBlueprintData::FBlueprintMetadata FBP2AIBatchExporter::ExportMetadata(UBlueprint* Blueprint)
{
    FCompleteBlueprintData::FBlueprintMetadata MD;
    if (!Blueprint) { return MD; }
    MD.ClassName = Blueprint->GetName();
    MD.ParentClass = Blueprint->ParentClass ? Blueprint->ParentClass->GetName() : TEXT("");
    MD.AssetPath = Blueprint->GetPathName();

    // 接口列表（UBlueprint::ImplementedInterfaces 是 FBPInterfaceDescription 数组）
    for (const FBPInterfaceDescription& Iface : Blueprint->ImplementedInterfaces)
    {
        UClass* InterfaceClass = Iface.Interface.Get();
        if (InterfaceClass) { MD.Interfaces.Add(InterfaceClass->GetName()); }
    }
    return MD;
}

TArray<FCompleteBlueprintData::FComponentInfo> FBP2AIBatchExporter::ExportComponents(UBlueprint* Blueprint)
{
    TArray<FCompleteBlueprintData::FComponentInfo> Out;
    if (!Blueprint) { return Out; }

    // 从生成的类的 CDO（类默认对象）获取所有组件（包括继承的）
    UBlueprintGeneratedClass* BPGC = Cast<UBlueprintGeneratedClass>(Blueprint->GeneratedClass);
    if (BPGC)
    {
        AActor* CDO = Cast<AActor>(BPGC->GetDefaultObject());
        if (CDO)
        {
            // 获取所有组件
            TInlineComponentArray<UActorComponent*> AllComponents;
            CDO->GetComponents(AllComponents);
            
            // 构建组件名称映射
            TMap<UActorComponent*, FString> ComponentNameMap;
            for (UActorComponent* Comp : AllComponents)
            {
                if (Comp)
                {
                    ComponentNameMap.Add(Comp, Comp->GetFName().ToString());
                }
            }

            // 导出所有组件及其父子关系
            for (UActorComponent* Comp : AllComponents)
            {
                if (!Comp) { continue; }
                
                FCompleteBlueprintData::FComponentInfo Info;
                Info.Name = Comp->GetFName().ToString();
                Info.Type = Comp->GetClass()->GetName();
                Info.ParentName = TEXT("");
                
                // 查找父组件（仅对 SceneComponent 有效）
                USceneComponent* SceneComp = Cast<USceneComponent>(Comp);
                if (SceneComp)
                {
                    USceneComponent* AttachParent = SceneComp->GetAttachParent();
                    if (AttachParent && ComponentNameMap.Contains(AttachParent))
                    {
                        Info.ParentName = ComponentNameMap[AttachParent];
                    }
                }
                
                Out.Add(Info);
            }
        }
    }

    // 补充并修正 SimpleConstructionScript 中的组件父子关系
    if (Blueprint->SimpleConstructionScript)
    {
        TSet<FString> ExistingNames;
        for (const auto& C : Out) { ExistingNames.Add(C.Name); }

        // 收集所有 SCS 节点的父子关系（包括指向 C++ 组件的引用）
        TMap<FString, FString> SCSParentMap; // Child -> Parent
        
        auto CollectSCSRelations = [&](USCS_Node* Node, const FString& ParentName, auto&& CollectRef) -> void
        {
            if (!Node) { return; }
            FString NodeName = Node->GetVariableName().ToString();
            
            // 关键修复：检查 SCS 节点的 ParentComponentOrVariableName
            // 这个字段包含了父组件的名称（可能是 C++ 继承的组件）
            FName ParentCompName = Node->ParentComponentOrVariableName;
            if (ParentCompName != NAME_None)
            {
                // 找到了显式的父组件名称（可能指向 C++ 组件）
                SCSParentMap.Add(NodeName, ParentCompName.ToString());
            }
            else if (!ParentName.IsEmpty())
            {
                // 使用传递下来的父节点名称
                SCSParentMap.Add(NodeName, ParentName);
            }
            
            // 递归处理子节点
            for (USCS_Node* Child : Node->GetChildNodes())
            {
                CollectRef(Child, NodeName, CollectRef);
            }
        };

        const TArray<USCS_Node*>& RootNodes = Blueprint->SimpleConstructionScript->GetRootNodes();
        for (USCS_Node* Root : RootNodes)
        {
            CollectSCSRelations(Root, TEXT(""), CollectSCSRelations);
        }

        // 更新已有组件的父子关系（SCS 的信息更准确）
        for (auto& C : Out)
        {
            if (SCSParentMap.Contains(C.Name))
            {
                C.ParentName = SCSParentMap[C.Name];
            }
        }

        // 添加 SCS 中存在但 CDO 中缺失的组件
        auto AddNodeRecursive = [&](USCS_Node* Node, auto&& AddNodeRecursiveRef) -> void
        {
            if (!Node) { return; }
            
            FString NodeName = Node->GetVariableName().ToString();
            if (!ExistingNames.Contains(NodeName))
            {
                FCompleteBlueprintData::FComponentInfo Info;
                Info.Name = NodeName;
                Info.Type = Node->ComponentClass ? Node->ComponentClass->GetName() : TEXT("UnknownComponentClass");
                
                // 使用已经收集的父子关系
                if (SCSParentMap.Contains(NodeName))
                {
                    Info.ParentName = SCSParentMap[NodeName];
                }
                else
                {
                    Info.ParentName = TEXT("");
                }
                
                Out.Add(Info);
                ExistingNames.Add(NodeName);
            }

            // 递归处理子节点
            for (USCS_Node* Child : Node->GetChildNodes())
            {
                AddNodeRecursiveRef(Child, AddNodeRecursiveRef);
            }
        };

        for (USCS_Node* Root : RootNodes)
        {
            AddNodeRecursive(Root, AddNodeRecursive);
        }
    }

    return Out;
}

static FString GetPinTypeDescription(const FEdGraphPinType& PinType)
{
    FString TypeStr = PinType.PinCategory.ToString();
    if (PinType.PinSubCategoryObject.IsValid())
    {
        TypeStr = PinType.PinSubCategoryObject->GetName();
    }
    else if (PinType.PinCategory == TEXT("object") || PinType.PinCategory == TEXT("class") || PinType.PinCategory == TEXT("interface"))
    {
        if (PinType.PinSubCategory != NAME_None)
        {
            TypeStr = PinType.PinSubCategory.ToString();
        }
    }
    
    if (PinType.ContainerType == EPinContainerType::Array)
    {
        TypeStr += TEXT("[]");
    }
    else if (PinType.ContainerType == EPinContainerType::Set)
    {
        TypeStr = FString::Printf(TEXT("Set<%s>"), *TypeStr);
    }
    else if (PinType.ContainerType == EPinContainerType::Map)
    {
        FString ValueType = TEXT("Unknown");
        if (PinType.PinValueType.TerminalCategory != NAME_None)
        {
             ValueType = PinType.PinValueType.TerminalCategory.ToString();
             if (PinType.PinValueType.TerminalSubCategoryObject.IsValid())
             {
                 ValueType = PinType.PinValueType.TerminalSubCategoryObject->GetName();
             }
        }
        TypeStr = FString::Printf(TEXT("Map<%s, %s>"), *TypeStr, *ValueType);
    }
    
    return TypeStr;
}

TArray<FCompleteBlueprintData::FVariableInfo> FBP2AIBatchExporter::ExportVariables(UBlueprint* Blueprint)
{
    TArray<FCompleteBlueprintData::FVariableInfo> Vars;
    if (!Blueprint) { return Vars; }
    for (const FBPVariableDescription& VarDesc : Blueprint->NewVariables)
    {
        FCompleteBlueprintData::FVariableInfo V;
        V.Name = VarDesc.VarName.ToString();
        // Use GetPinTypeDescription to correctly handle arrays, sets, maps, and object types
        V.Type = GetPinTypeDescription(VarDesc.VarType);
        V.bIsPublic = (VarDesc.PropertyFlags & CPF_Edit) != 0;
        V.Tooltip = VarDesc.FriendlyName;
        // 默认值（简单类型，复杂类型略过）
        V.DefaultValue = VarDesc.DefaultValue;
        Vars.Add(V);
    }
    return Vars;
}

TArray<FCompleteBlueprintData::FFunctionInfo> FBP2AIBatchExporter::ExportFunctions(UBlueprint* Blueprint)
{
    TArray<FCompleteBlueprintData::FFunctionInfo> Funcs;
    if (!Blueprint) { return Funcs; }

    TSet<FName> InterfaceFuncs = GetInterfaceFunctionNames(Blueprint);

    // Gather all function graphs, including interface graphs that may not reside in FunctionGraphs
    TArray<UEdGraph*> AllFunctionGraphs = Blueprint->FunctionGraphs;
    TSet<UEdGraph*> SeenFunctionGraphs;
    for (UEdGraph* Existing : AllFunctionGraphs)
    {
        if (Existing)
        {
            SeenFunctionGraphs.Add(Existing);
        }
    }
    for (const FBPInterfaceDescription& Iface : Blueprint->ImplementedInterfaces)
    {
        for (UEdGraph* InterfaceGraph : Iface.Graphs)
        {
            if (InterfaceGraph && !SeenFunctionGraphs.Contains(InterfaceGraph))
            {
                SeenFunctionGraphs.Add(InterfaceGraph);
                AllFunctionGraphs.Add(InterfaceGraph);
            }
        }
    }

    // 函数图表（不含事件）
    for (UEdGraph* FuncGraph : AllFunctionGraphs)
    {
        if (!FuncGraph) { continue; }
        FCompleteBlueprintData::FFunctionInfo FI;
        FI.Name = FuncGraph->GetName();
        FI.bIsEvent = false;
        FI.bIsPure = false;

        // 解析入口节点获取参数
        TArray<UK2Node_FunctionEntry*> EntryNodes;
        FuncGraph->GetNodesOfClass(EntryNodes);
        if (EntryNodes.Num() > 0)
        {
            UK2Node_FunctionEntry* EntryNode = EntryNodes[0];
            
            // 尝试从 SkeletonClass 获取函数纯度信息
            if (UClass* SkelClass = Blueprint->SkeletonGeneratedClass)
            {
                if (UFunction* Func = SkelClass->FindFunctionByName(FuncGraph->GetFName()))
                {
                    FI.bIsPure = Func->HasAnyFunctionFlags(FUNC_BlueprintPure);
                }
            }
            // 如果找不到 UFunction (极少情况)，尝试检查 EntryNode 的 ExtraFlags
            else
            {
                 FI.bIsPure = (EntryNode->GetExtraFlags() & FUNC_BlueprintPure) != 0;
            }

            for (UEdGraphPin* Pin : EntryNode->Pins)
            {
                // 入口节点的输出引脚即为函数参数
                if (Pin->Direction == EGPD_Output && !Pin->bHidden && 
                    Pin->PinType.PinCategory != TEXT("exec") && 
                    Pin->PinType.PinCategory != TEXT("then"))
                {
                    FCompleteBlueprintData::FFunctionParam Param;
                    Param.Name = Pin->PinName.ToString();
                    Param.Type = GetPinTypeDescription(Pin->PinType);
                    Param.bIsReturn = false;
                    FI.Parameters.Add(Param);
                }
            }
        }

        // 解析结果节点获取返回值
        TArray<UK2Node_FunctionResult*> ResultNodes;
        FuncGraph->GetNodesOfClass(ResultNodes);
        if (ResultNodes.Num() > 0)
        {
            UK2Node_FunctionResult* ResultNode = ResultNodes[0];
            for (UEdGraphPin* Pin : ResultNode->Pins)
            {
                // 结果节点的输入引脚即为返回值
                if (Pin->Direction == EGPD_Input && !Pin->bHidden && 
                    Pin->PinType.PinCategory != TEXT("exec") && 
                    Pin->PinType.PinCategory != TEXT("then"))
                {
                    FString TypeStr = GetPinTypeDescription(Pin->PinType);
                    if (FI.ReturnType.IsEmpty())
                    {
                        FI.ReturnType = TypeStr;
                    }
                    else
                    {
                        FI.ReturnType += TEXT(", ") + TypeStr;
                    }
                }
            }
        }
        
        if (FI.ReturnType.IsEmpty())
        {
            FI.ReturnType = TEXT("void");
        }

        // Check if this function is an interface implementation
        if (IsInterfaceGraph(FuncGraph, Blueprint, InterfaceFuncs))
        {
            FI.Name += TEXT(" (Interface Implementation)");
        }

        Funcs.Add(FI);
    }

    // 注意：不再将 EventGraph 添加到 Functions 列表，
    // 它们将在 Graph Logic 部分作为独立的事件图表展示。

    // --- Also extract event nodes from Ubergraph pages and add them to the functions list
    for (UEdGraph* EventGraph : Blueprint->UbergraphPages)
    {
        if (!EventGraph) { continue; }

        // Collect standard event nodes
        TArray<UK2Node_Event*> EventNodes;
        EventGraph->GetNodesOfClass(EventNodes);
        for (UK2Node_Event* EventNode : EventNodes)
        {
            FCompleteBlueprintData::FFunctionInfo FI;
            // Use a human-friendly title for the event (e.g. "EnhancedInputAction IA_Look")
            FI.Name = EventNode->GetNodeTitle(ENodeTitleType::ListView).ToString();
            FI.bIsEvent = true;
            FI.bIsPure = false;
            FI.ReturnType = TEXT("void");

            // Check if event implements an interface function
            // For events, the MemberName in EventReference usually matches the interface function name
            FName EventMemberName = EventNode->EventReference.GetMemberName();
            if (InterfaceFuncs.Contains(EventMemberName))
            {
                FI.Name += TEXT(" (Interface Implementation)");
            }

            // Parse event parameters from output pins
            for (UEdGraphPin* Pin : EventNode->Pins)
            {
                if (Pin->Direction == EGPD_Output && !Pin->bHidden &&
                    Pin->PinType.PinCategory != TEXT("exec") &&
                    Pin->PinType.PinCategory != TEXT("then") &&
                    Pin->PinType.PinCategory != TEXT("delegate"))
                {
                    FCompleteBlueprintData::FFunctionParam Param;
                    Param.Name = Pin->PinName.ToString();
                    Param.Type = GetPinTypeDescription(Pin->PinType);
                    Param.bIsReturn = false;
                    FI.Parameters.Add(Param);
                }
            }
            Funcs.Add(FI);
        }

        // Note: UK2Node_CustomEvent is a subclass of UK2Node_Event, so they are already collected above.
        // We do not need a separate loop for CustomEvents to avoid duplicates.
    }

    return Funcs;
}

static bool IsInterfaceGraph(UEdGraph* Graph, UBlueprint* Blueprint, const TSet<FName>& InterfaceFuncs)
{
    if (!Graph || !Blueprint)
    {
        return false;
    }

    // 1) Interface descriptions explicitly list their graphs, so check pointer equality first.
    for (const FBPInterfaceDescription& Iface : Blueprint->ImplementedInterfaces)
    {
        for (UEdGraph* InterfaceGraph : Iface.Graphs)
        {
            if (InterfaceGraph == Graph)
            {
                return true;
            }
        }
    }

    // 2) Check the entry node's referenced member name if available.
    TArray<UK2Node_FunctionEntry*> EntryNodes;
    Graph->GetNodesOfClass(EntryNodes);
    if (EntryNodes.Num() > 0)
    {
        const UK2Node_FunctionEntry* Entry = EntryNodes[0];
        const FName EntryMemberName = Entry->FunctionReference.GetMemberName();
        if (InterfaceFuncs.Contains(EntryMemberName))
        {
            return true;
        }
    }

    // 3) Direct graph name match.
    if (InterfaceFuncs.Contains(Graph->GetFName()))
    {
        return true;
    }

    // 4) Fallback: interface graphs commonly use "Interface_Function" naming. Check for suffix match.
    const FString GraphName = Graph->GetName();
    for (const FName& InterfaceFuncName : InterfaceFuncs)
    {
        const FString FuncString = InterfaceFuncName.ToString();
        if (GraphName.EndsWith(FuncString, ESearchCase::IgnoreCase) || GraphName.Contains(FuncString))
        {
            return true;
        }
    }

    return false;
}

FString FCompleteBlueprintData::ToMarkdown() const
{
    FString Result;
    Result += FString::Printf(TEXT("# %s\n\n"), *BlueprintName);
    Result += FString::Printf(TEXT("**Asset Path**: `%s`\n\n"), *AssetPath);
    Result += TEXT("---\n\n");
    auto AppendFunctionList = [&](const TArray<FCompleteBlueprintData::FFunctionInfo>& InFunctions)
    {
        if (InFunctions.Num() == 0)
        {
            Result += TEXT("(None)\n");
            return;
        }

        for (const auto& F : InFunctions)
        {
            Result += FString::Printf(TEXT("- %s%s"), *F.Name, F.bIsEvent ? TEXT(" (Event)") : TEXT(""));
            if (!F.ReturnType.IsEmpty())
            {
                Result += FString::Printf(TEXT(" -> %s"), *F.ReturnType);
            }
            if (F.Parameters.Num() > 0)
            {
                Result += TEXT(" ( ");
                for (int32 i = 0; i < F.Parameters.Num(); ++i)
                {
                    const auto& P = F.Parameters[i];
                    Result += FString::Printf(TEXT("%s: %s"), *P.Name, *P.Type);
                    if (i < F.Parameters.Num() - 1) { Result += TEXT(", "); }
                }
                Result += TEXT(" )");
            }
            Result += TEXT("\n");
        }
    };

    if (bIsInterface)
    {
        Result += TEXT("## Interface Functions\n\n");
        AppendFunctionList(Functions);
        Result += TEXT("\n---\n\n");
        return Result;
    }
    
    // Summary section removed by user request
    /*
    int32 TotalNodes = 0, TotalLines = 0, TotalBlocks = 0;
    for (const FExportedGraphInfo& G : Graphs)
    {
        TotalNodes += G.NodeCount;
        TotalLines += G.LineCount;
        TotalBlocks += G.BlueprintBlockCount;
    }
    Result += TEXT("## Summary\n\n");
    Result += FString::Printf(TEXT("- **Total Graphs**: %d\n"), Graphs.Num());
    Result += FString::Printf(TEXT("- **Total Nodes**: %d\n"), TotalNodes);
    Result += FString::Printf(TEXT("- **Total Blueprint Blocks**: %d\n"), TotalBlocks);
    Result += FString::Printf(TEXT("- **Total Lines**: %d\n"), TotalLines);
    Result += TEXT("\n---\n\n");
    */

    // Metadata（新）
    Result += TEXT("## Metadata\n\n");
    Result += FString::Printf(TEXT("- **Class**: %s\n"), *Metadata.ClassName);
    Result += FString::Printf(TEXT("- **ParentClass**: %s\n"), *Metadata.ParentClass);
    if (Metadata.Interfaces.Num() > 0)
    {
        Result += TEXT("- **Interfaces**:\n");
        for (const FString& I : Metadata.Interfaces)
        {
            Result += FString::Printf(TEXT("  - %s\n"), *I);
        }
    }
    Result += TEXT("\n---\n\n");

    // Components（新）
    Result += TEXT("## Components\n\n");
    for (const auto& C : Components)
    {
        Result += FString::Printf(TEXT("- %s : %s"), *C.Name, *C.Type);
        if (!C.ParentName.IsEmpty())
        {
            Result += FString::Printf(TEXT(" (Parent: %s)"), *C.ParentName);
        }
        Result += TEXT("\n");
    }
    if (Components.Num() == 0) { Result += TEXT("(None)\n"); }
    Result += TEXT("\n---\n\n");

    // Variables（新）
    Result += TEXT("## Variables\n\n");
    for (const auto& V : Variables)
    {
        Result += FString::Printf(TEXT("- %s : %s"), *V.Name, *V.Type);
        if (!V.DefaultValue.IsEmpty()) { Result += FString::Printf(TEXT(" = %s"), *V.DefaultValue); }
        if (V.bIsPublic) { Result += TEXT(" (Public)"); }
        if (!V.Tooltip.IsEmpty()) { Result += FString::Printf(TEXT(" // %s"), *V.Tooltip); }
        Result += TEXT("\n");
    }
    if (Variables.Num() == 0) { Result += TEXT("(None)\n"); }
    Result += TEXT("\n---\n\n");

    // Functions（新）
    Result += TEXT("## Functions\n\n");
    AppendFunctionList(Functions);
    Result += TEXT("\n---\n\n");

    // Graph Inventory（保留）
    Result += TEXT("## Graph Inventory\n\n");
    TMap<FString, int32> CategoryCount;
    for (const FExportedGraphInfo& G : Graphs) { CategoryCount.FindOrAdd(G.Category)++; }
    for (const auto& Pair : CategoryCount)
    {
        Result += FString::Printf(TEXT("- **%s**: %d\n"), *Pair.Key, Pair.Value);
    }
    Result += TEXT("\n---\n\n");

    // Graph Logic（保留）
    Result += TEXT("## Graph Logic\n\n");
    for (const FExportedGraphInfo& G : Graphs)
    {
        // For delegates we only emit a header (no code blocks or trace content)
        if (G.Category.Equals(TEXT("Delegate"), ESearchCase::IgnoreCase))
        {
            Result += FString::Printf(TEXT("### [%s] %s\n\n"), *G.Category, *G.GraphName);
            Result += TEXT("\n---\n\n");
            continue;
        }

        Result += FString::Printf(TEXT("### [%s] %s\n\n"), *G.Category, *G.GraphName);
        // Stats removed by user request
        /*
        Result += FString::Printf(TEXT("- **Nodes**: %d\n"), G.NodeCount);
        Result += FString::Printf(TEXT("- **Lines**: %d\n"), G.LineCount);
        Result += FString::Printf(TEXT("- **Blueprint Blocks**: %d\n\n"), G.BlueprintBlockCount);
        */
        Result += G.Markdown;
        Result += TEXT("\n\n---\n\n");
    }

    return Result;
}

FCompleteBlueprintData FBP2AIBatchExporter::ExportCompleteBlueprint(UBlueprint* Blueprint, bool bIncludeNestedFunctions)
{
    FCompleteBlueprintData Result;
    if (!Blueprint)
    {
        UE_LOG(LogBP2AI, Error, TEXT("ExportCompleteBlueprint: Blueprint is null"));
        return Result;
    }

    Result.BlueprintName = Blueprint->GetName();
    Result.AssetPath = Blueprint->GetPathName();
    Result.Metadata   = ExportMetadata(Blueprint);
    Result.bIsInterface = (Blueprint->BlueprintType == BPTYPE_Interface);

    if (Result.bIsInterface)
    {
        UE_LOG(LogBP2AI, Log, TEXT("ExportCompleteBlueprint: '%s' detected as Blueprint Interface"), *Result.BlueprintName);
        Result.Functions = ExportInterfaceFunctionSignatures(Blueprint);
        WriteBlueprintMarkdown(Result);
        return Result;
    }

    // 图表
    FBP2AIBatchExporter Exporter;
    Result.Graphs = Exporter.ExportAllGraphsDetailed(Blueprint, bIncludeNestedFunctions);

    // 阶段3：元数据
    Result.Components = ExportComponents(Blueprint);
    Result.Variables  = ExportVariables(Blueprint);
    Result.Functions  = ExportFunctions(Blueprint);

    // 写出蓝图级 Markdown 文档
    WriteBlueprintMarkdown(Result);

    return Result;
}

// 定义：蓝图级 Markdown 写出
static bool WriteBlueprintMarkdown(const FCompleteBlueprintData& Data)
{
    FString BaseDir = FPaths::ProjectSavedDir() / TEXT("BP2AI/Exports");
    IFileManager::Get().MakeDirectory(*BaseDir, true);
    FString FileName = Data.BlueprintName + TEXT(".md");
    FString FilePath = BaseDir / FileName;
    FString Content = Data.ToMarkdown();
    if (FFileHelper::SaveStringToFile(Content, *FilePath))
    {
        UE_LOG(LogBP2AI, Log, TEXT("📘 Saved blueprint document: %s"), *FilePath);
        return true;
    }
    UE_LOG(LogBP2AI, Warning, TEXT("⚠️ Failed to save blueprint document: %s"), *FilePath);
    return false;
}
