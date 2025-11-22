# 🎯 Task 1.3 测试流程快速参考

## 测试流程图

```
┌─────────────────────────────────────────────────────────────┐
│  第一步：准备测试蓝图                                        │
│  ------------------------------------------------           │
│  在 UE 编辑器中创建：                                       │
│  路径: Content/Test/BP_TestExport                           │
│  类型: Actor Blueprint                                      │
│  内容: BeginPlay → Print String                             │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│  第二步：编译插件                                           │
│  ------------------------------------------------           │
│  方法 A: Rider 中 Build → Rebuild Module "BP2AI"            │
│  方法 B: UE 编辑器自动提示编译，点击 "Yes"                  │
│  验证: ✅ Build succeeded, 0 errors                          │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│  第三步：打开测试界面                                        │
│  ------------------------------------------------           │
│  1. 在 Content Browser 搜索 "PropertyDumper"                │
│  2. 确保已勾选 "Show Plugin Content"                        │
│  3. 双击打开 EUW_PropertyDumper Widget                      │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│  第四步：执行测试                                           │
│  ------------------------------------------------           │
│  在 PropertyDumper Widget 中：                              │
│  → 点击 "Run Current Phase Test" 按钮                       │
│  → Widget 显示: "Executing current phase test..."          │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│  第五步：查看结果                                           │
│  ------------------------------------------------           │
│  1. 打开 Output Log (Window → Developer Tools)              │
│  2. 搜索框输入: "BP2AI"                                     │
│  3. 查找以下成功标志:                                       │
│     ✅ "Blueprint loaded successfully"                       │
│     ✅ "Exported 'EventGraph': XXX characters"               │
│     ✅ "TEST PASSED: Successfully exported X graphs"         │
│     ✅ "TASK 1.3 TEST PASSED!"                               │
└─────────────────────────────────────────────────────────────┘
```

---

## 测试代码工作原理

### **调用链**

```cpp
用户点击 "Run Current Phase Test" 按钮
    ↓
PropertyDumperWidget::OnRunCurrentPhaseTestClicked()
    ↓
BP2AITests::ExecuteCurrentPhaseTest()
    ↓
FBP2AIBatchExporter::TestExportBlueprint("/Game/Test/BP_TestExport")
    ↓
LoadObject<UBlueprint>() - 加载蓝图
    ↓
FBP2AIBatchExporter::ExportAllGraphsFromBlueprint()
    ↓
遍历蓝图的所有图表：
    - UbergraphPages (Event Graphs)
    - FunctionGraphs (Functions)
    - MacroGraphs (Macros)
    - DelegateSignatureGraphs (Delegates)
    ↓
对每个图表调用：ExportSingleGraph()
    ↓
获取图表所有节点：Graph->Nodes
    ↓
调用 BP2AI 核心：FExecutionFlowGenerator::GenerateDocumentForNodes()
    ↓
返回 Markdown 文档
    ↓
记录日志并统计结果
```

---

## 关键文件位置

```
测试入口:
Source/BP2AI/Private/Test/CurrentPhaseTest.cpp
    └─ ExecuteCurrentPhaseTest() 函数
         └─ 第 31 行: FString TestBlueprintPath

导出器实现:
Source/BP2AI/Private/Exporters/BP2AIBatchExporter.cpp
    ├─ TestExportBlueprint() - 静态测试方法
    ├─ ExportAllGraphsFromBlueprint() - 导出整个蓝图
    └─ ExportSingleGraph() - 导出单个图表

UI 触发器:
Source/BP2AI/Private/Widgets/PropertyDumperWidget.cpp
    └─ OnRunCurrentPhaseTestClicked() 函数
```

---

## 常见问题快速解决

| 问题 | 解决方案 |
|------|----------|
| 🔴 "Blueprint is null" | 检查路径格式，确保蓝图已保存 |
| 🔴 "No graphs exported" | 确保蓝图 Event Graph 中有节点 |
| 🔴 找不到 PropertyDumper | 启用 "Show Plugin Content" |
| 🔴 按钮无反应 | 检查 Output Log 是否过滤了 LogBP2AI |
| 🔴 编译错误 | 删除 Intermediate 和 Binaries 文件夹，重新生成项目文件 |

---

## 成功标志检查清单

- [ ] Output Log 中看到 `🧪 BP2AI BATCH EXPORTER TEST - Task 1.3`
- [ ] 看到 `✅ Blueprint loaded successfully: BP_TestExport`
- [ ] 看到 `BP2AIBatchExporter: Starting export for graph 'EventGraph'`
- [ ] 看到 `✅ Exported 'EventGraph': XXX characters, XX lines`
- [ ] 看到统计信息：`Total Graphs: X, Event Graphs: X`
- [ ] 最后看到 `✅ TASK 1.3 TEST PASSED!`

---

## 下一步行动

测试通过后：

1. **更新进度**
   - 打开 `TASK_ROADMAP.md`
   - 标记 Task 1.3 为完成 ✅
   - 记录完成日期和导出的图表数量

2. **截图保存**
   - 保存 Output Log 的成功输出
   - 作为 Task 1.3 完成的证据

3. **准备 Task 2.1**
   - 查看 TASK_ROADMAP.md 中的 Task 2.1 要求
   - 开始研究如何获取蓝图元数据

---

**详细步骤** → 查看 [`TESTING_GUIDE.md`](TESTING_GUIDE.md)

