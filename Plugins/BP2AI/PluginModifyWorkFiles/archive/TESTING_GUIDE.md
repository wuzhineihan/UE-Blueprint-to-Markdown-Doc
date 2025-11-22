# 🧪 BP2AI Task 1.3 详细测试指南

**测试目标**: 验证 `FBP2AIBatchExporter` 能够程序化地导出蓝图

---

## 📋 测试流程概览

```
准备测试蓝图
    ↓
编译插件
    ↓
打开 UE 编辑器
    ↓
打开 PropertyDumper Widget
    ↓
点击 "Run Current Phase Test" 按钮
    ↓
查看 Output Log 验证结果
```

---

## 🎯 第一步：准备测试蓝图

### **方法 A：创建新的测试蓝图（推荐）**

1. **打开 UE 编辑器**
   - 打开你的项目：`CPPPlayGround.uproject`

2. **创建测试文件夹**
   - 在 Content Browser 中，右键点击 `Content` 文件夹
   - 选择 `New Folder`
   - 命名为 `Test`

3. **创建测试蓝图**
   - 在 `Content/Test` 文件夹中右键
   - 选择 `Blueprint Class`
   - 选择父类 `Actor`
   - 命名为 `BP_TestExport`

4. **添加简单内容**（让蓝图有数据可导出）
   - 双击打开 `BP_TestExport`
   - 在 Event Graph 中：
     - 找到 `Event BeginPlay` 节点（如果没有，右键搜索添加）
     - 拖出执行引脚，搜索 `Print String`
     - 连接 `BeginPlay → Print String`
     - 在 Print String 的 `In String` 输入框输入：`"Test Export Working!"`
   - **保存并关闭**蓝图

5. **验证路径**
   - 在 Content Browser 中右键点击 `BP_TestExport`
   - 选择 `Copy Reference`
   - 应该得到类似：`Blueprint'/Game/Test/BP_TestExport.BP_TestExport'`
   - **测试路径**就是：`/Game/Test/BP_TestExport`

### **方法 B：使用现有蓝图**

如果你项目中已有蓝图，可以直接使用：

1. 在 Content Browser 中找到任意蓝图
2. 右键 → `Copy Reference`
3. 获得路径如：`Blueprint'/Game/MyFolder/BP_MyActor.BP_MyActor'`
4. 测试路径就是：`/Game/MyFolder/BP_MyActor`（去掉后缀和类型前缀）

### **路径格式说明**

✅ **正确格式**:
```
/Game/Test/BP_TestExport
/Game/Characters/BP_Player
/Game/Weapons/BP_Pistol
```

❌ **错误格式**:
```
Blueprint'/Game/Test/BP_TestExport.BP_TestExport'  // 不要包含类型前缀
/Game/Test/BP_TestExport_C                          // 不要加 _C 后缀
C:/Content/Test/BP_TestExport                       // 不要用文件系统路径
```

---

## 🔧 第二步：修改测试路径（可选）

如果你的蓝图路径不是 `/Game/Test/BP_TestExport`，需要修改测试代码：

1. **打开文件**: `Source/BP2AI/Private/Test/CurrentPhaseTest.cpp`

2. **找到第 31 行**:
   ```cpp
   FString TestBlueprintPath = TEXT("/Game/Test/BP_TestExport");
   ```

3. **修改为你的蓝图路径**:
   ```cpp
   FString TestBlueprintPath = TEXT("/Game/YourFolder/YourBlueprint");
   ```

4. **保存文件**

---

## 🏗️ 第三步：编译插件

### **在 Rider 中编译**:

1. 打开 Rider
2. 打开项目解决方案（`.sln` 文件）
3. 在 Solution Explorer 中找到 `BP2AI` 插件
4. 右键 → `Build` 或 `Rebuild`
5. 等待编译完成（查看 Build 窗口输出）

### **通过 UE 编辑器自动编译**:

1. 打开 UE 编辑器
2. 如果代码有变化，编辑器会提示：
   ```
   "Would you like to rebuild BP2AI now?"
   ```
3. 点击 **"Yes"**
4. 等待编译完成

### **编译成功标志**:
```
✅ Build succeeded
✅ 0 errors, 0 warnings
```

---

## 🎮 第四步：打开 PropertyDumper Widget

### **4.1 查找 PropertyDumper Widget**

1. **打开 UE 编辑器**
2. **在 Content Browser 中搜索**:
   - 点击 Content Browser 右上角的设置按钮（齿轮图标）
   - 确保勾选 `Show Plugin Content`（显示插件内容）
   - 在搜索框输入：`PropertyDumper`
   - 应该能看到 `EUW_PropertyDumper` 或类似名称的 Editor Utility Widget

3. **如果找不到，手动定位**:
   - 展开 Content Browser 左侧的文件夹树
   - 找到 `Plugins → BP2AI Content → Widgets`
   - 查找 `PropertyDumper` 相关的 Widget

### **4.2 打开 Widget**

1. **双击** `EUW_PropertyDumper`
2. Widget 会在编辑器中作为一个面板打开
3. 你应该看到一个界面，包含：
   - 文本输入框
   - 几个按钮
   - **重要**: 找到名为 **"Run Current Phase Test"** 的按钮

### **4.3 如果没有找到 Widget**

可以创建一个新的：

1. 在 Content Browser 中右键
2. 选择 `Editor Utilities → Editor Utility Widget`
3. 命名为 `EUW_PropertyDumper`
4. 打开后，在 Designer 中添加：
   - 一个 `Button`，命名为 `RunCurrentPhaseTestButton`
   - 一个 `MultiLineEditableTextBox`，命名为 `OutputResultsTextBox`
5. 编译并保存

---

## 🚀 第五步：执行测试

### **5.1 点击测试按钮**

1. 在 PropertyDumper Widget 界面中
2. 找到 **"Run Current Phase Test"** 按钮
3. **点击按钮**

### **5.2 观察反馈**

点击后，Widget 的文本框会显示：
```
Executing current phase test (2025.11.14-12:30:45)... Check Output Log.
```

然后变为：
```
Current phase test execution finished (2025.11.14-12:30:46). See Output Log for details.
```

---

## 📊 第六步：查看测试结果

### **6.1 打开 Output Log**

1. 在 UE 编辑器菜单栏：
   - `Window → Developer Tools → Output Log`
2. 或使用快捷键：`Ctrl + Shift + L`

### **6.2 过滤日志**

1. 在 Output Log 窗口顶部的搜索框输入：`BP2AI`
2. 或点击 `Categories` 按钮，勾选 `LogBP2AI`

### **6.3 解读测试结果**

#### **✅ 测试成功的日志输出**:

```
LogBP2AI: Warning: ========================================
LogBP2AI: Warning: 🧪 BP2AI BATCH EXPORTER TEST - Task 1.3
LogBP2AI: Warning: ========================================
LogBP2AI: Warning: 📝 Testing automatic blueprint export...
LogBP2AI: Warning:    Target Blueprint: /Game/Test/BP_TestExport
LogBP2AI: Warning:    (If this path doesn't exist, create a blueprint at this location)
LogBP2AI: Log: 🧪 TEST: Attempting to load blueprint: /Game/Test/BP_TestExport
LogBP2AI: Log: ✅ Blueprint loaded successfully: BP_TestExport
LogBP2AI: Log: ========================================
LogBP2AI: Log: BP2AIBatchExporter: Exporting blueprint 'BP_TestExport'
LogBP2AI: Log: ========================================
LogBP2AI: Log: 📊 Phase 1: Exporting Event Graphs
LogBP2AI: Log: BP2AIBatchExporter: Starting export for graph 'EventGraph' (3 nodes)
LogBP2AI: Log: ✅ Exported 'EventGraph': 456 characters, 23 lines
LogBP2AI: Log: 📊 Phase 2: Exporting Function Graphs
LogBP2AI: Log: 📊 Phase 3: Exporting Macro Graphs
LogBP2AI: Log: 📊 Phase 4: Exporting Delegate Graphs
LogBP2AI: Log: ========================================
LogBP2AI: Log: ✅ Export Complete:
LogBP2AI: Log:    Blueprint: BP_TestExport
LogBP2AI: Log:    Total Graphs: 1
LogBP2AI: Log:    Event Graphs: 1
LogBP2AI: Log:    Function Graphs: 0
LogBP2AI: Log:    Macro Graphs: 0
LogBP2AI: Log:    Delegate Graphs: 0
LogBP2AI: Log: ========================================
LogBP2AI: Log: ✅ TEST PASSED: Successfully exported 1 graphs
LogBP2AI: Warning: ========================================
LogBP2AI: Warning: ✅ TASK 1.3 TEST PASSED!
LogBP2AI: Warning:    We can now programmatically export blueprints!
LogBP2AI: Warning: ========================================
```

**成功标志**:
- ✅ `Blueprint loaded successfully`
- ✅ `Exported 'EventGraph': XXX characters`
- ✅ `TEST PASSED: Successfully exported X graphs`
- ✅ `TASK 1.3 TEST PASSED!`

#### **❌ 测试失败的日志输出**:

```
LogBP2AI: Error: ❌ TEST FAILED: Could not load blueprint at path: /Game/Test/BP_TestExport
LogBP2AI: Error: ========================================
LogBP2AI: Error: ❌ TASK 1.3 TEST FAILED
LogBP2AI: Error:    Please create a test blueprint and update the path
LogBP2AI: Error: ========================================
```

**失败原因**:
- ❌ 蓝图不存在
- ❌ 路径格式错误
- ❌ 蓝图是空的（没有任何图表）

---

## 🔧 故障排除

### **问题 1: "Blueprint is null" 错误**

**症状**:
```
LogBP2AI: Error: ❌ TEST FAILED: Could not load blueprint at path: /Game/Test/BP_TestExport
```

**解决方案**:
1. 确认蓝图已创建并保存
2. 验证路径格式正确（参考第一步）
3. 在 Content Browser 中右键蓝图 → `Copy Reference`，确认路径
4. 修改 `CurrentPhaseTest.cpp` 中的路径
5. 重新编译并测试

### **问题 2: "No graphs exported" 警告**

**症状**:
```
LogBP2AI: Warning: ⚠️ TEST WARNING: No graphs exported (blueprint might be empty)
```

**解决方案**:
1. 打开测试蓝图
2. 确保 Event Graph 中至少有一个节点（如 BeginPlay）
3. 保存蓝图
4. 重新运行测试

### **问题 3: 找不到 PropertyDumper Widget**

**解决方案**:
1. 确保在 Content Browser 中启用了 `Show Plugin Content`
2. 手动搜索 `PropertyDumper`
3. 如果还是找不到，可以直接在 C++ 代码中调用测试：

**方法 A: 通过蓝图调用**:
```cpp
// 在任意 Blueprint 的 Event Graph 中
// 添加一个 "Execute Console Command" 节点
// 命令输入: BP2AI.RunTest
```

**方法 B: 通过 C++ 直接调用**:
创建一个临时的 Blueprint Function Library：

```cpp
// 在你的项目中创建一个新的 C++ 类
#include "Test/CurrentPhaseTest.h"

UFUNCTION(BlueprintCallable, Category="BP2AI")
static void RunBP2AITest()
{
    BP2AITests::ExecuteCurrentPhaseTest();
}
```

### **问题 4: 编译错误**

**症状**:
```
Error: Cannot find FExecutionFlowGenerator
Error: Cannot find FGenerationSettings
```

**解决方案**:
1. 确保 BP2AI 插件已启用
2. 清理项目：
   - 关闭 UE 编辑器
   - 删除 `Intermediate` 和 `Binaries` 文件夹
   - 右键 `.uproject` → `Generate Visual Studio project files`
3. 重新编译

### **问题 5: 按钮点击无反应**

**症状**: 点击按钮后没有任何输出

**解决方案**:
1. 检查 Output Log 是否打开
2. 确认 `LogBP2AI` 日志类别未被过滤
3. 尝试重启 UE 编辑器

---

## 📈 测试成功后的意义

如果测试通过，说明我们成功实现了：

✅ **程序化加载蓝图** - 不需要手动打开编辑器  
✅ **自动获取图表** - 不需要用户选择节点  
✅ **调用 BP2AI 核心** - 复用现有功能生成文档  
✅ **批量处理能力** - 为后续批量导出打下基础  

**这是整个项目的基石**！

---

## 🎯 测试后的下一步

### **立即做**:
1. 截图保存测试成功的日志
2. 更新 `TASK_ROADMAP.md`:
   ```markdown
   - [x] **Task 1.3** - 编写自动化导出测试函数 ✅
     - 测试通过日期: 2025-11-14
     - 成功导出图表数量: X
   ```

### **准备 Task 2.1**:
- 研究如何获取蓝图的元数据（类名、父类、变量等）
- 在 `FBP2AIBatchExporter` 中添加新方法
- 继续扩展导出功能

---

## 💡 快速测试检查清单

- [ ] 测试蓝图已创建：`/Game/Test/BP_TestExport`
- [ ] 蓝图中至少有一个节点（BeginPlay → Print String）
- [ ] 插件已编译成功（无错误）
- [ ] PropertyDumper Widget 已找到
- [ ] Output Log 已打开
- [ ] 点击 "Run Current Phase Test" 按钮
- [ ] 查看日志中的 `BP2AI` 输出
- [ ] 验证看到 `TEST PASSED` 消息

---

**准备好了？** 开始测试吧！🚀

如有问题，查看 `TECHNICAL_NOTES.md` 中的问题记录部分。

