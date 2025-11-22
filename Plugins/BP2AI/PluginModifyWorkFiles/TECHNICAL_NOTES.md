# BP2AI 技术分析笔记

**最后更新**: 2025-11-15

---

## 🔄 更新摘要（2025-11-15）

- 在 `FBP2AIBatchExporter::LogExportResult` 中新增 ExecFlow 预览日志输出：
  - 解析文档中首个 ```blueprint 代码块；若没有则回退到全文
  - 仅显示前 10 行；每行最长 200 字符，超出截断并标注 `...[truncated]`
  - 输出用清晰分隔符包裹：
    - `----- ExecFlow Preview for Graph: <GraphName> (showing N/M lines) -----`
    - 行格式：`[nn] <line>`
    - 末尾如被截断，输出：`... ExecFlow preview truncated: K more line(s) not shown...`
    - `----- End ExecFlow Preview for Graph: <GraphName> -----`
  - 目的：无需打开 Markdown 文件，即可在 Output Log 中快速人工核对执行流结构
- 保持导出文件写入 `Saved/BP2AI/Exports/*.md`，便于离线查看与比对
- 阶段切换：Stage 1 / Task 1.3 完成，进入 Stage 2 / Task 2.1（获取蓝图的所有图表）

---

## 🧩 关键实现细节：ExecFlow 预览

- 位置：`Plugins/BP2AI/Source/BP2AI/Private/Exporters/BP2AIBatchExporter.cpp`
- 函数：`void FBP2AIBatchExporter::LogExportResult(const FString& GraphName, const FString& Content) const`
- 核心逻辑（伪码）：
  ```cpp
  if (Content.IsEmpty()) { log("<no content>"); return; }
  const int32 MaxSearchWindow = 50000;
  const FString& SearchSource = Content.Len() > MaxSearchWindow ? Content.Left(MaxSearchWindow) : Content;
  // Try fenced block
  FenceStart = SearchSource.Find("```blueprint");
  if (FenceStart != INDEX_NONE) { BlockStart = findFirstNewlineAfter(FenceStart); FenceEnd = findNext("```", BlockStart+1); }
  FString Block = (BlockStart != INDEX_NONE && FenceEnd != INDEX_NONE) ? Mid(BlockStart+1, FenceEnd-(BlockStart+1)) : SearchSource;
  Block.ParseIntoArrayLines(BlockLines);
  Show first 10 lines; truncate per line at 200 chars; log with header/footer and truncation notice.
  ```
- 安全与健壮：
  - 大文档仅扫描前 50k 字符；找不到 fenced block 时回退全文
  - 空内容或无行时输出友好提示
  - 仅日志输出，不影响导出流程；失败/异常不致崩溃
- 参数（当前为常量）：
  - `DefaultPreviewLineCount = 10`
  - `MaxPreviewLineLength = 200`
  - `bPreviewContent = true`

> 后续可选：将上述参数改为控制台变量（如 `bp2ai.ExecFlowPreviewLines`），以便无需重编即可调节。

---

## 📐 当前调用链与测试路径（复核）

- Editor Utility Widget Button → 蓝图 `BP2AITestLibrary.TestExportBlueprintByPath` →
- C++ `UBP2AITestLibrary::TestExportBlueprintByPath` →
- `FBP2AIBatchExporter::TestExportBlueprint` →
- `FBP2AIBatchExporter::ExportAllGraphsFromBlueprint` →
- `FBP2AIBatchExporter::ExportSingleGraph` →
- `FExecutionFlowGenerator::GenerateDocumentForNodes` → Markdown →
- `FBP2AIBatchExporter::LogExportResult`（新增 ExecFlow 预览）

验证：在 Output Log 过滤 `LogBP2AI`，可见每个 Graph 的统计与预览。

---

## 📌 关键类与方法（摘录，便于 Stage 2 使用）

- `UBlueprint` 常用图表集合：
  - `UbergraphPages`（事件图表）
  - `FunctionGraphs`（函数图表）
  - `MacroGraphs`（宏图表）
  - `DelegateSignatureGraphs`（委托图表）
- `UEdGraph::Nodes` 获取图表中的所有节点
- `FExecutionFlowGenerator::GenerateDocumentForNodes(...)` 生成 Markdown/HTML
- `FGenerationSettings` 影响生成行为（追踪范围、是否分开定义用户图表、是否显示默认参数等）
- `FMarkdownGenerationContext(FMarkdownGenerationContext::EOutputFormat::RawMarkdown)` 指定输出 Markdown

---

## 🧭 Stage 2 启动指引（Task 2.1）

- 目标：打印并核对一个 Blueprint 的所有图表名称（Event/Function/Macro/Delegate）
- 建议在 `FBP2AIBatchExporter::ExportAllGraphsFromBlueprint` 中先打印清单：
  - Event: `Blueprint->UbergraphPages`
  - Function: `Blueprint->FunctionGraphs`
  - Macro: `Blueprint->MacroGraphs`
  - Delegate: `Blueprint->DelegateSignatureGraphs`
- 日志模板：
  ```text
  🔎 Graph Inventory for <BP>:
    - [Event ] <GraphName>
    - [Func  ] <GraphName>
    - [Macro ] <GraphName>
    - [Dele  ] <GraphName>
  ```
- 通过已有的 Editor 按钮继续调用 `TestExportBlueprintByPath`，观察清单与后续导出结果是否一致。

---

## 🔍 历史记录（节选）

- 2025-01-14：完成 Task 1.1 / 1.2，明确生成器入口与节点输入模式
- 2025-11-15：完成 Task 1.3，自动化导出闭环跑通并加入 ExecFlow 预览日志

---

## ⚙️ 备忘清单

- 线程模型：所有操作在游戏线程执行
- 资产生命周期：`LoadObject<UBlueprint>` 返回的 `UBlueprint*` 由 GC 管理
- 记录类别：主日志类别为 `LogBP2AI`（可在 Output Log 过滤）
- 输出目录：`Saved/BP2AI/Exports/*.md`

---

## 📚 附：原理与 API 摘要（保留）

### **调用链路图**

```
用户点击工具栏按钮
    ↓
FBP2AIModule::GenerateExecFlowAction()
    ↓
创建 SWindow + SMarkdownOutputWindow
    ↓
用户点击 "Refresh Flow" 按钮
    ↓
SMarkdownOutputWindow::UpdateFromSelection()
    ↓
GetSelectedBlueprintNodes() - 获取编辑器选中的节点
    ↓
FExecutionFlowGenerator::GenerateDocumentForNodes()
    ↓
返回 Markdown/HTML 文本
    ↓
显示在 UI 中
```

---

## 📌 关键类与方法

### **1. `SMarkdownOutputWindow` (UI层)**
**位置**: `Source/BP2AI/Private/Widgets/SMarkdownOutputWindow.cpp`

**核心职责**: 
- 提供用户界面(刷新/复制/打开HTML按钮)
- 管理生成设置(追踪所有节点/分别定义函数等)
- 调用底层生成器

**关键方法**:
```cpp
TArray<UEdGraphNode*> GetSelectedBlueprintNodes() const
```
**工作原理**:
1. 通过 `GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()` 获取资产编辑器子系统
2. 遍历所有打开的资产,找到最近激活的蓝图编辑器
3. 调用 `FBlueprintEditor::GetSelectedNodes()` 获取选中的节点集合
4. 将 `FGraphPanelSelectionSet` 转换为 `TArray<UEdGraphNode*>`

**重要发现**: 
- ✅ BP2AI 不依赖鼠标坐标或UI事件
- ✅ 它只需要 `UEdGraphNode*` 数组作为输入
- ✅ 这意味着我们可以**程序化地**传入任意节点数组!

---

### **2. `FExecutionFlowGenerator` (核心生成器)**
**位置**: `Source/BP2AI/Private/Trace/ExecutionFlow/ExecutionFlowGenerator.h`

**核心方法**:
```cpp
FString GenerateDocumentForNodes(
    const TArray<UEdGraphNode*>& SelectedNodes,
    const FGenerationSettings& Settings,
    FMarkdownGenerationContext& Context
);
```

**输入参数**:
- `SelectedNodes`: 要分析的节点数组 (可以是图表中的全部节点!)
- `Settings`: 生成配置(是否追踪所有执行路径/是否内联展开等)
- `Context`: 输出格式上下文(Markdown/HTML)

**输出**: 
- 格式化的 Markdown 或 HTML 字符串

**重要性**: 
- 这是我们复用 BP2AI 逻辑的**入口点**!
- 我们不需要重写节点追踪逻辑
- 只需要正确准备输入参数

---

### **3. `FGenerationSettings` (配置结构)**
**位置**: `Source/BP2AI/Private/Trace/Generation/GenerationShared.h`

**关键字段**:
```cpp
struct FGenerationSettings
{
    bool bTraceAllSelected = false;           // 追踪所有选中的执行节点
    bool bDefineUserGraphsSeparately = true;  // 分别定义用户函数/宏
    bool bExpandCompositesInline = false;     // 内联展开折叠图表
    bool bShowTrivialDefaultParams = true;    // 显示默认参数值
    bool bShouldTraceSymbolicallyForData = false;
    bool bUseSemanticData = false;
    TMap<EDocumentationGraphCategory, bool> CategoryVisibility; // 类别可见性
    
    // 注意：构造函数会自动初始化所有 CategoryVisibility
};
```

---

## 💡 关键洞察

### **我们可以复用 BP2AI 的方式**

BP2AI 的核心是**节点数组处理器**,而不是UI工具!

**这意味着**:
```cpp
// ✅ 我们可以这样做:
UBlueprint* Blueprint = ...;
UEdGraph* EventGraph = Blueprint->UbergraphPages[0];

// 获取图表中的所有节点(不需要用户选中!)
TArray<UEdGraphNode*> AllNodes = EventGraph->Nodes;

// 直接调用 BP2AI 的生成器
FGenerationSettings Settings;
Settings.bTraceAllSelectedExec = true;
Settings.bDefineUserGraphsSeparately = true;

FMarkdownGenerationContext Context(
    FMarkdownGenerationContext::EOutputFormat::RawMarkdown
);

FExecutionFlowGenerator Generator;
FString GraphLogic = Generator.GenerateDocumentForNodes(
    AllNodes, 
    Settings, 
    Context
);

// ✅ GraphLogic 现在包含了整个图表的逻辑!
```

### **这解决了什么问题**

原来的 BP2AI:
- ❌ 需要用户手动选中节点
- ❌ 一次只能导出一个图表
- ❌ 无法批量处理

我们的方案:
- ✅ 程序化获取所有节点
- ✅ 自动遍历所有图表
- ✅ 支持批量导出整个项目

---

## 🎯 下一步实现计划

### **Task 2.1 的预研结论**

要获取蓝图的所有图表,我们需要访问:

```cpp
UBlueprint* Blueprint = ...;

// 1. 事件图表 (EventGraph)
TArray<UEdGraph*> EventGraphs = Blueprint->UbergraphPages;

// 2. 函数图表
TArray<UEdGraph*> FunctionGraphs = Blueprint->FunctionGraphs;

// 3. 宏图表
TArray<UEdGraph*> MacroGraphs = Blueprint->MacroGraphs;

// 4. 委托图表
TArray<UEdGraph*> DelegateSignatureGraphs = Blueprint->DelegateSignatureGraphs;

// 对每个图表:
for (UEdGraph* Graph : EventGraphs)
{
    FString GraphName = Graph->GetName();
    TArray<UEdGraphNode*> AllNodes = Graph->Nodes;
    
    // 调用 BP2AI 导出
    FString GraphLogic = ExportGraphLogic(AllNodes);
    
    // 存储结果
    MyBlueprintData.GraphLogics.Add(GraphName, GraphLogic);
}
```

### **已验证的假设**

✅ `UBlueprint` 包含所有图表的引用  
✅ `UEdGraph::Nodes` 包含图表中的所有节点  
✅ `FExecutionFlowGenerator` 接受任意节点数组  
✅ 不需要编辑器状态或用户交互  

---

## 📦 index.json（阶段4预热）

- 位置：Saved/BP2AI/Exports/<BlueprintName>__index.json
- 由来：阶段 2 / Task 2.2 中，详细导出（ExportAllGraphsDetailed）完成后写出，用于：
  - 快速自动化核对（Graphs/Nodes/Lines/Blocks 等统计）
  - 为阶段 4 的统一数据结构与多格式输出做验证与预热
- 内容结构：
  ```json
  {
    "BlueprintName": "BP_TestExport",
    "TotalGraphs": 6,
    "TotalNodes": 123,
    "TotalCharacters": 9876,
    "TotalLines": 456,
    "TotalBlueprintBlocks": 21,
    "Graphs": [
      {
        "GraphName": "EventGraph",
        "Category": "Event",
        "NodeCount": 24,
        "CharacterCount": 1234,
        "LineCount": 82,
        "BlueprintBlockCount": 9
      }
      // ... others ...
    ]
  }
  ```
- 生成职责：
  - 由导出器类 `FBP2AIBatchExporter` 提供静态方法 `WriteIndexJson(...)` 写出
  - 测试入口（`UBP2AITestLibrary::TestExportBlueprintByPath`）仅调用导出器接口，不直接写文件（职责内聚在导出器）
- 与 Markdown 的关系：
  - Markdown 仍是每图的“人类可读”主格式，index.json 用于统计与结构化概览
  - 最终阶段 4 将引入统一数据结构，支持 JSON/Markdown/HTML 并存输出

---

## 🐛 问题记录与解决方案

### **问题 1: FGenerationSettings 字段名称错误 (2025-01-14)**

**错误**: 使用了不存在的 `bTraceAllSelectedExec` 字段
```cpp
Settings.bTraceAllSelectedExec = true;  // ❌ 错误：字段不存在
```

**原因**: `FGenerationSettings` 结构体中的正确字段名是 `bTraceAllSelected`

**解决方案**:
```cpp
Settings.bTraceAllSelected = true;  // ✅ 正确
```

**位置**: `BP2AIBatchExporter.cpp` 第 34 行

**重要发现**:
- `FGenerationSettings` 构造函数已经自动初始化了 `CategoryVisibility`
- 所有布尔字段都有默认值，可以选择性覆盖
- 结构体定义在 `Trace/Generation/GenerationShared.h` 第 121 行

---

### **解决方案 2: PropertyDumper Widget 打不开问题 (2025-11-14)**

**问题**: PropertyDumper 是 C++ 类而非 UMG Widget，双击会跳转到 Rider 源代码

**原因**: `UPropertyDumperWidget` 是一个 C++ 基类，需要基于它创建蓝图 Widget 才能使用

**解决方案**: 创建 `BP2AITestLibrary` 蓝图函数库，提供更简单的测试方法

**新增文件**:
- `Source/BP2AI/Public/BP2AITestLibrary.h` - 蓝图函数库头文件
- `Source/BP2AI/Private/BP2AITestLibrary.cpp` - 实现文件

**核心函数**:
```cpp
UFUNCTION(BlueprintCallable, Category = "BP2AI|Testing", meta = (CallInEditor = "true"))
static void RunTask13Test();
```

**使用方法**:
1. 创建任意蓝图（Actor 或 Editor Utility Widget）
2. 在 Event Graph 中右键搜索 `Run Task13 Test`
3. 连接到 BeginPlay 或按钮点击事件
4. 运行蓝图或点击按钮即可执行测试

**优势**:
- ✅ 不需要创建复杂的 UMG Widget
- ✅ 可以在编辑器中直接调用（`CallInEditor = "true"`）
- ✅ 可以在任意蓝图中使用
- ✅ 支持 Python 脚本调用：`unreal.BP2AITestLibrary.run_task13_test()`

**详细步骤**: 参见 `SIMPLE_TESTING_GUIDE.md`

---

## 🐛 问题记录与解决方案（补充 2025-11-15）

### 问题 3：EventGraph 在 Output Log 仅显示一个事件（遗漏其他事件）

现象：
- 测试图表结构中 EventGraph 含 3 个事件（IA_Look / IA_Move / IA_Jump），但 Output Log 的 ExecFlow 预览只显示了其中 1 个事件的蓝图块。

排查与结论：
- 检查保存的 Markdown：`Saved/BP2AI/Exports/BP_TestExport__EventGraph_EventGraph.md` 包含多个独立的 ```blueprint 代码块（每个事件各占一个 block），说明“导出逻辑正确”。
- 根因：旧版日志预览逻辑只解析并打印了“首个” ```blueprint 代码块，未遍历后续块，导致日志中看起来像是“只导出一个事件”。

修复：
- 在 `FBP2AIBatchExporter::LogExportResult` 中改为“扫描所有 ```blueprint 块”，并预览最多 3 个 block，每个 block 打印前 6 行（可配置常量）。
- 预览头部会显示 `blocks: showing X/Y`，并为每个 block 打印一行简短标题（取首个非空行）。
- 若 block 数或行数超限，会打印清晰的截断提示。

验证方法：
1. 重新编译并运行测试按钮。
2. 在 Output Log 过滤 `LogBP2AI`，查找 `----- ExecFlow Preview for Graph: EventGraph (blocks: showing X/Y) -----`。
3. 应能看到 `[Block 1/…] …`、`[Block 2/…] …`、`[Block 3/…] …` 等多段事件预览。
4. 与保存的 Markdown 内容逐一比对，确认三个事件可见。

备注：
- 保存的 Markdown 为 UTF-16（平台默认基于 TCHAR）；在非 UTF-16 友好的查看器里会看到“空字节”的显示，这是正常的编码表现，不影响导出或日志预览。
