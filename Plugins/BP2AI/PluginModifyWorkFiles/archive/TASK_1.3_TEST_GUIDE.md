# Task 1.3 测试指南

**目标**: 验证我们可以程序化地导出蓝图,不依赖 UI

---

## 📋 准备工作

### 1. 确认新文件已创建

检查这些文件是否存在:
```
✅ Source/BP2AI/Public/Exporters/BP2AIBatchExporter.h
✅ Source/BP2AI/Private/Exporters/BP2AIBatchExporter.cpp
✅ Source/BP2AI/Private/Test/CurrentPhaseTest.cpp (已更新)
```

### 2. 在你的 UE 项目中创建测试蓝图

1. 打开 Unreal Editor
2. 在 Content Browser 中创建文件夹: `Content/Test/`
3. 右键 → Blueprint Class → Actor
4. 命名为: `BP_TestExport`
5. 打开蓝图并添加简单内容:
   - 在 EventGraph 中添加 `BeginPlay` 事件
   - 连接一个 `Print String` 节点
   - 保存并关闭

---

## 🔧 编译插件

### 方法 1: 通过 UE 编辑器
1. 关闭 Unreal Editor (如果打开)
2. 右键点击你的 `.uproject` 文件
3. 选择 "Generate Visual Studio project files"
4. 打开生成的 `.sln` 文件
5. 在 Visual Studio 中,找到 BP2AI 插件
6. 右键 BP2AI → Build
7. 等待编译完成

### 方法 2: 通过编辑器自动编译
1. 直接打开 Unreal Editor
2. 编辑器会自动检测到插件代码变化
3. 点击 "Yes" 重新编译插件

---

## 🧪 执行测试

### 步骤 1: 打开 PropertyDumper Widget

1. 在 Content Browser 中搜索 "PropertyDumper"
2. 找到 `EUW_PropertyDumper` (Editor Utility Widget)
3. 双击打开

### 步骤 2: 运行测试

1. 在 PropertyDumper 窗口中找到 **"Run Test"** 按钮
2. 点击按钮
3. 测试会自动执行

### 步骤 3: 查看结果

#### 在 PropertyDumper 窗口中
- 会显示测试结果的摘要
- ✅ 表示测试通过
- ❌ 表示测试失败 (可能需要修改蓝图路径)

#### 在 Output Log 中
1. 打开: Window → Developer Tools → Output Log
2. 在过滤器中输入: `BP2AI`
3. 查看详细的导出日志

---

## 📊 预期输出

### ✅ 成功的输出日志示例:

```
LogBP2AI: 🧪 TEST: Attempting to load blueprint: /Game/Test/BP_TestExport
LogBP2AI: ✅ Blueprint loaded successfully: BP_TestExport
LogBP2AI: ========================================
LogBP2AI: BP2AIBatchExporter: Exporting blueprint 'BP_TestExport'
LogBP2AI: ========================================
LogBP2AI: 📊 Phase 1: Exporting Event Graphs
LogBP2AI: BP2AIBatchExporter: Starting export for graph 'EventGraph' (3 nodes)
LogBP2AI: ✅ Exported 'EventGraph': 456 characters, 23 lines
LogBP2AI: 📊 Phase 2: Exporting Function Graphs
LogBP2AI: 📊 Phase 3: Exporting Macro Graphs
LogBP2AI: 📊 Phase 4: Exporting Delegate Graphs
LogBP2AI: ========================================
LogBP2AI: ✅ Export Complete:
LogBP2AI:    Blueprint: BP_TestExport
LogBP2AI:    Total Graphs: 1
LogBP2AI:    Event Graphs: 1
LogBP2AI:    Function Graphs: 0
LogBP2AI:    Macro Graphs: 0
LogBP2AI:    Delegate Graphs: 0
LogBP2AI: ========================================
LogBP2AI: ✅ TEST PASSED: Successfully exported 1 graphs
```

---

## ❌ 故障排除

### 问题 1: "Blueprint is null" 错误
**原因**: 蓝图路径不正确或蓝图不存在

**解决方法**:
1. 确认蓝图已创建并保存
2. 检查路径是否正确 (区分大小写!)
3. 更新 `CurrentPhaseTest.cpp` 第 25 行的路径:
   ```cpp
   FString TestBlueprintPath = TEXT("/Game/Test/BP_TestExport");
   ```
4. 重新编译插件

### 问题 2: 编译错误
**常见错误**:
- "Cannot find FExecutionFlowGenerator"
- "Cannot find FGenerationSettings"

**解决方法**:
1. 确认 BP2AI 插件已启用
2. 清理项目: Delete `Intermediate/` 和 `Binaries/` 文件夹
3. 重新生成项目文件
4. 重新编译

### 问题 3: PropertyDumper Widget 找不到
**解决方法**:
1. 在 Content Browser 中右键 → Miscellaneous → Editor Utility Widget
2. 命名为 `EUW_PropertyDumper`
3. 打开后添加一个按钮,绑定到测试函数

---

## ✅ 测试成功标志

如果你看到以下内容,说明 Task 1.3 **完成**:

✅ **编译通过** - 没有编译错误  
✅ **测试通过** - Output Log 显示 "TEST PASSED"  
✅ **导出成功** - 能看到图表的字符数统计  
✅ **程序化调用** - 没有打开任何 BP2AI UI 窗口  

---

## 🎯 这证明了什么?

完成 Task 1.3 后,我们验证了:

1. ✅ **可以绕过 UI** - 不需要用户选中节点
2. ✅ **可以加载任意蓝图** - 通过路径程序化加载
3. ✅ **可以获取所有图表** - UbergraphPages、FunctionGraphs 等
4. ✅ **可以调用 BP2AI 核心** - FExecutionFlowGenerator 工作正常
5. ✅ **可以获取 Markdown 输出** - 返回完整的文档字符串

**下一步**: Task 2.1 - 实现获取蓝图元数据 (类名、父类、变量等)

---

## 📝 测试后更新进度

测试通过后,请告诉我:
- ✅ 测试是否通过
- 📊 导出了多少个图表
- 📄 生成的 Markdown 有多少字符
- 🐛 遇到了什么问题(如果有)

我会更新 `TASK_ROADMAP.md` 的进度!
