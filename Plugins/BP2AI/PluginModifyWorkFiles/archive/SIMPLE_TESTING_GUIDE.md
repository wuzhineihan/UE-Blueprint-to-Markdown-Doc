# 🚀 简化版测试指南 - 无需 PropertyDumper Widget

如果你打不开 PropertyDumper Widget（因为它是 C++ 类），使用这个更简单的方法！

---

## 方法 1：通过蓝图调用测试（最简单）⭐⭐⭐⭐⭐

### **步骤 1：创建测试蓝图**

1. 在 Content Browser 右键
2. 选择 `Blueprint Class → Actor`
3. 命名为 `BP_RunTest`
4. 双击打开

### **步骤 2：添加测试节点**

1. 在 Event Graph 中右键
2. 搜索 `Run Task13 Test`（这是我们刚创建的函数）
3. 添加该节点
4. 连接到 `BeginPlay` 事件：
   ```
   Event BeginPlay → Run Task13 Test
   ```
5. 编译并保存蓝图

### **步骤 3：运行测试**

1. 将 `BP_RunTest` 拖到关卡中
2. 点击编辑器工具栏的 **"Play"** 按钮（或按 Alt+P）
3. 游戏开始运行时，测试会自动执行
4. 按 Esc 停止游戏

### **步骤 4：查看结果**

1. 打开 Output Log（Window → Developer Tools → Output Log）
2. 搜索 `BP2AI`
3. 查找 `✅ TEST PASSED`

---

## 方法 2：通过蓝图编辑器直接调用（无需 Play）⭐⭐⭐⭐

### **步骤 1：创建 Editor Utility Blueprint**

1. 在 Content Browser 右键
2. 选择 `Editor Utilities → Editor Utility Widget`
3. 命名为 `EUW_TestRunner`
4. 双击打开

### **步骤 2：添加按钮**

1. 在 Designer 模式下：
   - 从 Palette 拖一个 `Button` 到画布
   - 在 Details 面板设置按钮文字为 "Run Test"

2. 切换到 Graph 模式

3. 选择按钮，在 Details 面板找到 `On Clicked` 事件
   - 点击 `+` 添加事件

4. 在事件图表中：
   - 右键搜索 `Run Task13 Test`
   - 连接 `On Clicked → Run Task13 Test`

5. 编译并保存

### **步骤 3：运行 Widget**

1. 在 Content Browser 中右键点击 `EUW_TestRunner`
2. 选择 `Run Editor Utility Widget`
3. Widget 窗口会弹出
4. 点击 "Run Test" 按钮

### **步骤 4：查看结果**

打开 Output Log，搜索 `BP2AI`

---

## 方法 3：通过 Python 脚本调用（高级）⭐⭐⭐

如果你启用了 Python 插件：

1. 打开 Python Console（Window → Developer Tools → Python Console）

2. 输入以下代码：
   ```python
   import unreal
   
   # 调用测试函数
   unreal.BP2AITestLibrary.run_task13_test()
   ```

3. 按 Enter 执行

4. 查看 Output Log

---

## 方法 4：创建测试用的目标蓝图（最推荐）⭐⭐⭐⭐⭐

### **实际上你需要做的是创建测试目标，而不是测试触发器！**

### **步骤 1：创建要被测试的蓝图**

1. Content Browser → 右键 → `New Folder` → 命名为 `Test`
2. 在 `Test` 文件夹中右键 → `Blueprint Class → Actor`
3. 命名为 `BP_TestExport`
4. 双击打开
5. 在 Event Graph 中添加简单内容：
   - `Event BeginPlay → Print String`（随便添加一些节点）
6. 保存

### **步骤 2：编译插件**

1. 关闭 UE 编辑器
2. 在 Rider 中：
   - 找到 BP2AI 插件模块
   - 右键 → Build 或 Rebuild
3. 等待编译完成

### **步骤 3：修改测试路径（如果需要）**

如果你的蓝图路径不是 `/Game/Test/BP_TestExport`：

1. 打开 `Source/BP2AI/Private/Test/CurrentPhaseTest.cpp`
2. 找到第 31 行：
   ```cpp
   FString TestBlueprintPath = TEXT("/Game/Test/BP_TestExport");
   ```
3. 改为你的蓝图路径
4. 重新编译

### **步骤 4：通过任意方法触发测试**

使用上面的方法 1、2 或 3 来触发测试

---

## 🎯 最简单的完整流程（推荐）

1. **创建测试目标蓝图**：
   ```
   Content/Test/BP_TestExport
   添加节点：BeginPlay → Print String
   ```

2. **创建测试触发蓝图**：
   ```
   Content/BP_RunTest (Actor)
   添加节点：BeginPlay → Run Task13 Test
   ```

3. **编译插件**（Rider 中 Build）

4. **运行测试**：
   - 拖 `BP_RunTest` 到关卡
   - 点击 Play

5. **查看结果**：
   - Output Log 搜索 `BP2AI`
   - 找到 `✅ TEST PASSED`

---

## 📊 预期的 Output Log 输出

```
LogBP2AI: Warning: ========================================
LogBP2AI: Warning: 🎮 BP2AI Test Library - Running Task 1.3 Test
LogBP2AI: Warning: ========================================
LogBP2AI: Warning: ========================================
LogBP2AI: Warning: 🧪 BP2AI BATCH EXPORTER TEST - Task 1.3
LogBP2AI: Warning: ========================================
LogBP2AI: Log: 🧪 TEST: Attempting to load blueprint: /Game/Test/BP_TestExport
LogBP2AI: Log: ✅ Blueprint loaded successfully: BP_TestExport
LogBP2AI: Log: ========================================
LogBP2AI: Log: BP2AIBatchExporter: Exporting blueprint 'BP_TestExport'
LogBP2AI: Log: ========================================
LogBP2AI: Log: 📊 Phase 1: Exporting Event Graphs
LogBP2AI: Log: ✅ Exported 'EventGraph': 456 characters, 23 lines
LogBP2AI: Log: ✅ Export Complete: Total Graphs: 1
LogBP2AI: Log: ✅ TEST PASSED: Successfully exported 1 graphs
LogBP2AI: Warning: ✅ Test execution completed. Check Output Log for results.
```

---

## ❓ 常见问题

### Q: 找不到 "Run Task13 Test" 节点

**A**: 确保：
1. 插件已编译成功
2. 在蓝图中右键搜索时，取消勾选 "Context Sensitive"
3. 搜索 `BP2AI` 查看所有相关函数

### Q: 测试运行了但没有输出

**A**: 
1. 确保 Output Log 已打开
2. 确保没有过滤掉 `LogBP2AI` 类别
3. 查看 `LogBP2AI` 或者清空所有过滤器

### Q: 蓝图加载失败

**A**:
1. 检查路径格式：`/Game/Folder/BlueprintName`（不要加 `_C` 后缀）
2. 确保蓝图已保存
3. 在 Content Browser 右键蓝图 → Copy Reference 查看正确路径

---

## ✅ 成功标志

- [ ] Output Log 显示 `🎮 BP2AI Test Library - Running Task 1.3 Test`
- [ ] 显示 `✅ Blueprint loaded successfully`
- [ ] 显示 `✅ Exported 'EventGraph': XXX characters`
- [ ] 显示 `✅ TEST PASSED`
- [ ] 没有红色错误信息

---

## 🎉 测试通过后

1. 截图保存 Output Log
2. 更新 `TASK_ROADMAP.md` 标记 Task 1.3 完成
3. 准备进入 Task 2.1

---

**这个方法不需要 PropertyDumper Widget，更简单！** 🚀

