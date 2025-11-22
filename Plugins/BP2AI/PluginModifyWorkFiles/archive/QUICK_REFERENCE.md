# 🚀 BP2AI 开发快速参考

**最后更新**: 2025-01-14  
**当前任务**: Task 1.3 - 自动化导出测试

---

## 📋 文档导航

| 文档 | 用途 | 什么时候看 |
|------|------|------------|
| `PROJECT_STATUS_FOR_RIDER.md` | **完整项目状态** | 🔥 新加入时必读 |
| `TASK_ROADMAP.md` | 21个任务清单 | 查看进度/选择下一个任务 |
| `TECHNICAL_NOTES.md` | BP2AI 技术原理 | 理解代码工作原理 |
| `TASK_1.3_TEST_GUIDE.md` | Task 1.3 测试步骤 | 当前任务的详细指南 |
| `conversation.txt` | 需求讨论 | 理解项目背景 |

---

## 🎯 当前状态一览

```
✅ Task 1.1 - 理解架构 (完成)
✅ Task 1.2 - 分析入口点 (完成)
🔄 Task 1.3 - 自动化导出测试 (代码完成,等待测试)
```

**进度**: 10% (2/21)  
**下一步**: 编译 → 测试 → Task 2.1

---

## 🔧 新增/修改的文件

### ✅ 新增
```
Source/BP2AI/Public/Exporters/BP2AIBatchExporter.h
Source/BP2AI/Private/Exporters/BP2AIBatchExporter.cpp
PROJECT_STATUS_FOR_RIDER.md (本项目状态文档)
TASK_1.3_TEST_GUIDE.md (测试指南)
TECHNICAL_NOTES.md (技术笔记)
```

### 📝 已修改
```
Source/BP2AI/Private/Test/CurrentPhaseTest.cpp (添加 Task 1.3 测试)
TASK_ROADMAP.md (更新进度)
```

### 📖 只读参考
```
Source/BP2AI/Private/Trace/ExecutionFlow/ExecutionFlowGenerator.h/.cpp
Source/BP2AI/Private/Widgets/SMarkdownOutputWindow.cpp
Source/BP2AI/Private/BP2AI.cpp
```

---

## 🧪 Task 1.3 测试快速指南

### 前置条件
```cpp
// 1. 创建测试蓝图
路径: /Game/Test/BP_TestExport
类型: Actor Blueprint
内容: BeginPlay → Print String

// 2. (可选) 修改测试路径
文件: CurrentPhaseTest.cpp:25
代码: FString TestBlueprintPath = TEXT("/Game/YourPath/YourBP");
```

### 执行测试
```
1. 编译 BP2AI 插件 (Rider: Build → Rebuild Module "BP2AI")
2. 打开 UE 编辑器
3. 打开 PropertyDumper Widget
4. 点击 "Run Test" 按钮
5. 查看 Output Log (搜索 "BP2AI")
```

### 成功标志
```
LogBP2AI: ✅ TEST PASSED: Successfully exported X graphs
```

---

## 💡 核心代码片段

### 导出单个图表
```cpp
#include "Exporters/BP2AIBatchExporter.h"

FBP2AIBatchExporter Exporter;
FString Markdown = Exporter.ExportSingleGraph(Graph);
```

### 导出整个蓝图
```cpp
UBlueprint* BP = LoadObject<UBlueprint>(nullptr, TEXT("/Game/Path/BP_Name"));
TMap<FString, FString> Results = Exporter.ExportAllGraphsFromBlueprint(BP);

for (const auto& Pair : Results)
{
    UE_LOG(LogBP2AI, Log, TEXT("%s: %d chars"), *Pair.Key, Pair.Value.Len());
}
```

### BP2AI 核心调用
```cpp
#include "Trace/ExecutionFlow/ExecutionFlowGenerator.h"
#include "Trace/MarkdownGenerationContext.h"

FExecutionFlowGenerator Generator;
FGenerationSettings Settings = CreateDefaultSettings();
FMarkdownGenerationContext Context(FMarkdownGenerationContext::EOutputFormat::RawMarkdown);

FString Output = Generator.GenerateDocumentForNodes(AllNodes, Settings, Context);
```

---

## 🔍 常用日志搜索

在 Output Log 中搜索:
```
"BP2AI"              → 所有相关日志
"TEST PASSED"        → 测试成功
"TEST FAILED"        → 测试失败
"Exported"           → 导出结果
"BP2AIBatchExporter" → 导出器日志
```

---

## 🐛 常见问题

### Q: 编译错误 "Cannot find FExecutionFlowGenerator"
**A**: 缺少头文件引用
```cpp
#include "Trace/ExecutionFlow/ExecutionFlowGenerator.h"
#include "Trace/MarkdownGenerationContext.h"
#include "Trace/Generation/GenerationShared.h"
```

### Q: "Blueprint is null" 错误
**A**: 蓝图路径错误或蓝图不存在
- 检查路径格式: `/Game/Folder/BP_Name` (不要加 `_C`)
- 确认蓝图已保存
- 更新 `CurrentPhaseTest.cpp` 第 25 行

### Q: PropertyDumper Widget 找不到
**A**: 可以直接在 C++ 中调用
```cpp
#include "Test/CurrentPhaseTest.h"
BP2AITests::ExecuteCurrentPhaseTest();
```

---

## 📞 下一步 (Task 1.3 通过后)

### Task 2.1: 研究图表获取
```cpp
// 验证代码
UBlueprint* BP = ...;
UE_LOG(LogBP2AI, Log, TEXT("EventGraphs: %d"), BP->UbergraphPages.Num());
UE_LOG(LogBP2AI, Log, TEXT("Functions: %d"), BP->FunctionGraphs.Num());
UE_LOG(LogBP2AI, Log, TEXT("Macros: %d"), BP->MacroGraphs.Num());
```

### Task 3.1: 导出元数据
```cpp
struct FBlueprintMetadata {
    FString ClassName;    // BP->GetName()
    FString ParentClass;  // BP->ParentClass->GetName()
    FString AssetPath;    // BP->GetPathName()
};
```

---

## ✅ Rider 中的快捷操作

```
Ctrl+Shift+F    → 全局搜索
Ctrl+Click      → 跳转到定义
Alt+Enter       → 快速修复
Ctrl+B          → 编译项目
Ctrl+Shift+B    → 重新编译
```

---

**需要更多信息?** 查看 `PROJECT_STATUS_FOR_RIDER.md` 📖
