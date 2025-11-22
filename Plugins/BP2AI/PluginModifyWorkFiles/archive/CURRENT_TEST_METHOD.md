# Task 1.3 测试方法（最简单）

## 🎯 使用 Editor Utility Widget 测试

### **步骤 1: 创建测试目标蓝图**
1. Content Browser → 创建文件夹 `Test`
2. 在 Test 文件夹创建 Actor Blueprint: `BP_TestExport`
3. 添加简单节点：BeginPlay → Print String
4. 保存

### **步骤 2: 创建 Editor Utility Widget**
1. Content Browser → 右键 → `Editor Utilities → Editor Utility Widget`
2. 命名：`EUW_BP2AITest`
3. 双击打开

### **步骤 3: 添加测试按钮**
1. **Designer 模式**：拖一个 Button 到画布
2. **Graph 模式**：选择 Button → Details → On Clicked → 点击 `+`
3. 在事件中右键搜索 `Run Task13 Test`
4. 连接：`On Clicked → Run Task13 Test`
5. 编译保存

### **步骤 4: 运行测试**
1. Content Browser → 右键 `EUW_BP2AITest` → `Run Editor Utility Widget`
2. 点击按钮
3. 查看 Output Log（Window → Developer Tools → Output Log）
4. 搜索 `BP2AI`

### **预期输出**
```
LogBP2AI: ✅ Blueprint loaded successfully: BP_TestExport
LogBP2AI: ✅ Exported 'EventGraph': XXX characters
LogBP2AI: ✅ TEST PASSED
```

## 📝 注意事项
- ✅ 无需编译插件（BP2AITestLibrary 已存在）
- ✅ 不需要 Play 模式（纯编辑器运行）
- ✅ 可以重复运行测试

## 🔧 如果找不到函数
确保在蓝图中搜索时**取消勾选** "Context Sensitive"

