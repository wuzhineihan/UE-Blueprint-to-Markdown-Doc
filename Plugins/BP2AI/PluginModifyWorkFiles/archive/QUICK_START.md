# 🚀 Task 1.3 测试 - 5分钟快速开始

## ⚡ 最快测试方法（3步搞定）

### 📝 **第一步：创建测试目标蓝图**（2分钟）

1. 打开 UE 编辑器
2. Content Browser → 右键 → `New Folder` → 命名 `Test`
3. 在 Test 文件夹中：右键 → `Blueprint Class` → 选择 `Actor`
4. 命名为 `BP_TestExport`
5. 双击打开，添加节点：
   ```
   Event BeginPlay → Print String (输入任意文字)
   ```
6. **保存并关闭**

---

### 🔧 **第二步：编译插件**（1分钟）

在 **Rider** 中：
1. 找到 `BP2AI` 模块
2. 右键 → `Build` 或 `Rebuild`
3. 等待编译完成（看到 ✅ Build succeeded）

---

### 🎮 **第三步：运行测试**（2分钟）

#### **方法 A：通过蓝图触发（推荐）**

1. Content Browser → 右键 → `Blueprint Class → Actor`
2. 命名为 `BP_RunTest`
3. 双击打开，在 Event Graph 中：
   - 右键搜索 `Run Task13 Test`（取消勾选 Context Sensitive）
   - 连接：`Event BeginPlay → Run Task13 Test`
4. 编译并保存
5. 将 `BP_RunTest` 拖到关卡中
6. 点击工具栏 **Play** 按钮（或 Alt+P）

#### **方法 B：通过 Editor Utility Widget**

1. Content Browser → 右键 → `Editor Utilities → Editor Utility Widget`
2. 命名为 `EUW_Test`
3. 添加一个 Button，在 `On Clicked` 事件中调用 `Run Task13 Test`
4. 右键 Widget → `Run Editor Utility Widget`
5. 点击按钮

---

### 📊 **查看结果**

1. **打开 Output Log**：`Window → Developer Tools → Output Log`
2. **搜索**：`BP2AI`
3. **查找成功标志**：

```
✅ Blueprint loaded successfully: BP_TestExport
✅ Exported 'EventGraph': XXX characters
✅ TEST PASSED: Successfully exported 1 graphs
```

---

## 🎯 完整流程图

```
创建 BP_TestExport           编译插件               创建 BP_RunTest
(测试目标)                 (Rider Build)         (测试触发器)
    ↓                          ↓                        ↓
添加 BeginPlay 节点        等待编译成功         添加 Run Task13 Test 节点
    ↓                          ↓                        ↓
保存                      返回 UE 编辑器           连接到 BeginPlay
                                                        ↓
                                              拖到关卡 → 点击 Play
                                                        ↓
                                              打开 Output Log 查看结果
```

---

## ⚠️ 常见问题

| 问题 | 解决方案 |
|------|----------|
| 找不到 `Run Task13 Test` 节点 | 1. 确保插件已编译<br>2. 右键搜索时取消勾选 "Context Sensitive"<br>3. 搜索 `BP2AI` 查看所有函数 |
| "Blueprint is null" 错误 | 1. 确认 BP_TestExport 已创建在 `/Game/Test/` 路径<br>2. 如果路径不同，修改 `CurrentPhaseTest.cpp` 第 31 行 |
| Output Log 没有输出 | 1. 确保 Output Log 已打开<br>2. 清空所有过滤器<br>3. 搜索 `BP2AI` |

---

## ✅ 成功检查清单

- [ ] BP_TestExport 已创建并有节点
- [ ] 插件编译成功（无错误）
- [ ] BP_RunTest 创建并连接了测试节点
- [ ] 拖到关卡并点击 Play
- [ ] Output Log 显示 `✅ TEST PASSED`

---

## 🎉 测试通过后做什么

1. **截图保存** Output Log 的成功信息
2. **更新进度**：在 `TASK_ROADMAP.md` 标记 Task 1.3 完成 ✅
3. **查看下一任务**：Task 2.1 - 研究如何获取蓝图的所有图表

---

## 📚 需要更详细的说明？

- **简化指南**：[SIMPLE_TESTING_GUIDE.md](SIMPLE_TESTING_GUIDE.md) - 4种测试方法
- **详细指南**：[TESTING_GUIDE.md](TESTING_GUIDE.md) - 完整步骤和故障排除
- **快速参考**：[QUICK_TEST_REFERENCE.md](QUICK_TEST_REFERENCE.md) - 流程图和代码说明

---

**开始测试！** 🚀 只需 5 分钟！

