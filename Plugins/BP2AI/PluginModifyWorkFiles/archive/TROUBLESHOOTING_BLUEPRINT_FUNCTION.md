# 🔍 蓝图中找不到 RunTask13Test 函数 - 排查指南

## 问题描述
在蓝图的 Event Graph 中右键搜索 `RunTask13Test` 或 `Run Task13 Test`，找不到这个函数。

---

## ✅ 排查步骤（按顺序执行）

### **步骤 1：确认插件已编译** ⭐⭐⭐⭐⭐

这是最常见的原因！新添加的 C++ 代码必须编译后才能在蓝图中使用。

#### **1.1 在 Rider 中编译**

1. 打开 Rider
2. 在 Solution Explorer 中找到 `BP2AI` 模块
3. 右键 → `Build` 或 `Rebuild`
4. 等待编译完成，查看输出：
   ```
   ✅ Build succeeded
   ✅ 0 errors, 0 warnings (或只有警告，没有错误)
   ```

#### **1.2 在 UE 编辑器中重新编译**

1. **关闭** UE 编辑器（如果打开着）
2. 重新打开 UE 编辑器
3. 编辑器会检测到代码变化，提示：
   ```
   "Would you like to rebuild BP2AI now?"
   ```
4. 点击 **"Yes"**
5. 等待编译完成

#### **1.3 完全重新编译（如果上面方法无效）**

1. **关闭** UE 编辑器
2. 删除以下文件夹：
   - `Plugins/BP2AI/Binaries/`
   - `Plugins/BP2AI/Intermediate/`
3. 右键 `.uproject` 文件 → `Generate Visual Studio project files`
4. 在 Rider 中重新打开项目
5. Build → Rebuild Solution
6. 重新打开 UE 编辑器

---

### **步骤 2：检查蓝图搜索设置** ⭐⭐⭐⭐

在蓝图 Event Graph 中搜索时，需要正确设置：

#### **2.1 取消"Context Sensitive"**

1. 在蓝图 Event Graph 中右键
2. 在搜索框右侧，找到 **"Context Sensitive"** 复选框
3. **取消勾选** 这个选项
4. 再次搜索 `Run Task13 Test` 或 `RunTask13Test`

#### **2.2 尝试不同的搜索关键词**

- `Run Task13 Test`（带空格，自动生成的显示名称）
- `RunTask13Test`（原始函数名）
- `BP2AI`（查看所有 BP2AI 相关函数）
- `Test`（查看所有测试函数）

#### **2.3 查看 Category**

在搜索结果中，函数应该显示在：
```
Category: BP2AI | Testing
```

---

### **步骤 3：检查 UHT（Unreal Header Tool）生成** ⭐⭐⭐

UE 使用 UHT 生成反射代码，确保生成正确：

#### **3.1 检查生成的头文件**

查找文件：
```
Plugins/BP2AI/Intermediate/Build/Win64/UnrealEditor/Inc/BP2AI/UHT/BP2AITestLibrary.gen.cpp
```

如果这个文件不存在或很旧，说明 UHT 没有运行。

#### **3.2 强制重新生成**

1. 关闭 UE 编辑器
2. 删除 `Plugins/BP2AI/Intermediate/` 文件夹
3. 在 Rider 中 Build
4. 重新打开 UE 编辑器

---

### **步骤 4：验证代码语法** ⭐⭐⭐

检查 `BP2AITestLibrary.h` 中的关键部分：

#### **必须有的元素**

✅ **UCLASS()** 宏
```cpp
UCLASS()
class BP2AI_API UBP2AITestLibrary : public UBlueprintFunctionLibrary
```

✅ **GENERATED_BODY()** 宏
```cpp
{
    GENERATED_BODY()
```

✅ **UFUNCTION()** 宏，带正确的参数
```cpp
UFUNCTION(BlueprintCallable, Category = "BP2AI|Testing", meta = (CallInEditor = "true"))
static void RunTask13Test();
```

✅ **.generated.h** 文件必须被包含（通过 .generated.h 后缀自动处理）
```cpp
#include "BP2AITestLibrary.generated.h"  // 必须在最后一个 include
```

---

### **步骤 5：检查插件是否启用** ⭐⭐

#### **5.1 在编辑器中检查**

1. UE 编辑器菜单：`Edit → Plugins`
2. 搜索 `BP2AI`
3. 确认插件已勾选（Enabled）
4. 如果刚勾选，需要重启编辑器

#### **5.2 检查 .uproject 文件**

打开项目根目录的 `.uproject` 文件，确认包含：
```json
{
    "Plugins": [
        {
            "Name": "BP2AI",
            "Enabled": true
        }
    ]
}
```

---

### **步骤 6：使用替代测试方法（临时解决）** ⭐

如果以上步骤都无效，可以先用替代方法测试：

#### **方法 A：直接修改现有蓝图事件**

在 `PropertyDumperWidget` 的 C++ 实现中已经有测试代码，你可以：
1. 创建一个简单的 Actor Blueprint
2. 在 BeginPlay 事件中添加 C++ 代码调用：
   ```cpp
   #include "Test/CurrentPhaseTest.h"
   BP2AITests::ExecuteCurrentPhaseTest();
   ```

#### **方法 B：使用 Editor Utility Blueprint**

1. 创建 Editor Utility Blueprint
2. 在其中直接写 C++ 节点或使用 Python

#### **方法 C：使用 Python 脚本**

如果启用了 Python 插件：
```python
import unreal
# 即使蓝图找不到，Python 可能可以访问
unreal.BP2AITestLibrary.run_task13_test()
```

---

## 🎯 最可能的原因（按概率排序）

| 原因 | 概率 | 解决方法 |
|------|------|---------|
| 1. 插件未编译 | 90% | 步骤 1 - 编译插件 |
| 2. Context Sensitive 已勾选 | 5% | 步骤 2.1 - 取消勾选 |
| 3. UHT 未重新生成 | 3% | 步骤 3 - 删除 Intermediate 文件夹 |
| 4. 编辑器缓存问题 | 1% | 重启编辑器 |
| 5. 代码语法错误 | 1% | 步骤 4 - 检查代码 |

---

## ✅ 验证函数是否可用

### **测试 1：搜索其他 BP2AI 函数**

在蓝图中搜索 `BP2AI`，看是否能找到其他函数。如果一个都找不到，说���是插件级别的问题。

### **测试 2：查看 C++ 类浏览器**

1. UE 编辑器菜单：`Window → Developer Tools → Class Viewer`
2. 在搜索框输入 `BP2AITestLibrary`
3. 如果找到这个类，右键 → `Show in Content Browser`
4. 查看类的详细信息

### **测试 3：使用 Blueprint Function Library**

创建一个测试蓝图：
1. 创建 Actor Blueprint
2. 在 Event Graph 中右键
3. 搜索 `BlueprintFunctionLibrary`
4. 看是否能找到其他标准的函数库（如 KismetMathLibrary）
5. 如果标准库也找不到，说明是蓝图搜索的全局问题

---

## 🚀 快速解决方案（推荐）

### **最快的修复步骤**

```
1. 关闭 UE 编辑器
2. 在 Rider 中：Build → Rebuild Module "BP2AI"
3. 等待编译成功（0 errors）
4. 打开 UE 编辑器
5. 创建测试蓝图
6. 右键 → 取消勾选 "Context Sensitive"
7. 搜索 "BP2AI" 或 "Run Task13 Test"
```

---

## 📊 成功标志

如果问题解决，你应该能在蓝图搜索中看到：

```
Run Task13 Test
Category: BP2AI | Testing
Target: BP2AITestLibrary
```

函数图标显示为紫色（表示是 Blueprint Function Library 中的静态函数）

---

## 🔧 最后的杀手锏（如果所有方法都失败）

### **方案 1：手动验证编译输出**

查看编译日志，确认 `BP2AITestLibrary.cpp` 被编译了：
```
Rider → Build 输出窗口
搜索：BP2AITestLibrary
应该看到：Compiling BP2AITestLibrary.cpp
```

### **方案 2：创建最小测试案例**

创建一个超简单的测试函数：
```cpp
UFUNCTION(BlueprintCallable)
static void TestFunction() { }
```

如果这个也找不到，说明是 UE 项目配置问题。

### **方案 3：检查模块依赖**

打开 `BP2AI.Build.cs`，确认包含必要的模块：
```csharp
PublicDependencyModuleNames.AddRange(new string[] { 
    "Core", 
    "CoreUObject", 
    "Engine",
    "UMG",           // 必须有，用于 BlueprintFunctionLibrary
    "Slate",
    "SlateCore"
});
```

---

## 💡 调试技巧

### **在 Output Log 中查看反射信息**

1. 打开 UE 编辑器
2. Output Log 中搜索：`BP2AITestLibrary`
3. 如果找不到任何相关日志，说明类没有被注册

### **使用 Class Viewer 验证**

如果在 Class Viewer 中能找到 `UBP2AITestLibrary`，但蓝图中找不到函数，可能是：
- `BlueprintCallable` 拼写错误
- Category 格式问题（不要用中文）
- meta 标签格式错误

---

## 📞 需要帮助？

如果尝试了所有步骤还是不行，提供以下信息：

1. Rider 编译输出（是否成功？有错误吗？）
2. UE 编辑器版本（5.4.x）
3. 在蓝图中搜索 `BP2AI` 能找到什么？
4. Output Log 中搜索 `BP2AITestLibrary` 的结果
5. Class Viewer 中能否找到 `UBP2AITestLibrary`？

---

**最常见的解决方案：关闭编辑器 → Rider 中 Rebuild → 重新打开编辑器** ✅

