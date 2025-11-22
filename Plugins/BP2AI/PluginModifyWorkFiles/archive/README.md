# BP2AI 插件开发文档

**项目目标**: 将 Unreal Engine 蓝图自动导出为 AI 可理解的结构化文档

---

## 📋 核心文档

### **QUICK_START.md** 🚀🔥 新手必看
- **用途**: 5分钟快速测试指南
- **何时查看**: 
  - **现在！想立即开始测试**
  - 打不开 PropertyDumper Widget
  - 不想看长篇文档
- **内容**: 3步完成测试，最简化流程

### **TASK_ROADMAP.md** ⭐⭐⭐⭐⭐
- **用途**: 任务清单和进度跟踪
- **何时查看**: 
  - 每天开始工作前，查看当前任务
  - 完成任务后，更新进度和状态
  - 需要调整未来任务规划时
- **维护**: 持续更新，实时反映项目进度

### **TECHNICAL_NOTES.md** ⭐⭐⭐⭐
- **用途**: 技术细节和开发参考
- **何时查看**:
  - 实现功能时，查找 API 用法
  - 遇到技术问题时，查找解决方案
  - 需要理解 BP2AI 工作原理时
- **维护**: 开发过程中记录重要技术发现

### **SIMPLE_TESTING_GUIDE.md** ⭐⭐⭐⭐⭐ 🔥推荐
- **用途**: 简化版测试指南（无需 PropertyDumper Widget）
- **何时查看**:
  - **打不开 PropertyDumper Widget 时**
  - 想要最简单的测试方法
  - 快速开始测试
- **内容**: 4种简单测试方法，推荐方法 1（通过蓝图调用）

### **TESTING_GUIDE.md** ⭐⭐⭐⭐
- **用途**: 详细的测试流程指南（使用 PropertyDumper Widget）
- **何时查看**:
  - 需要完整详细的步骤说明时
  - 遇到测试问题需要排查时
- **内容**: 完整的步骤说明、截图指引、故障排除

---

## 📂 文件结构

```
PluginModifyWorkFiles/
├── README.md                  # 本文件 - 文档导航
├── QUICK_START.md             # 🚀🔥 5分钟快速开始（新手必看）
├── TASK_ROADMAP.md            # 任务路线图（核心）
├── TECHNICAL_NOTES.md         # 技术笔记（核心）
├── SIMPLE_TESTING_GUIDE.md    # ✅ 简化测试指南（4种方法）
├── TESTING_GUIDE.md           # ✅ 详细测试指南（完整版）
├── QUICK_TEST_REFERENCE.md    # ⚡ 快速参考（流程图）
├── archive/                   # 已归档的历史文档
│   ├── PROJECT_STATUS_FOR_RIDER.md
│   ├── INDEX.md
│   ├── QUICK_REFERENCE.md
│   ├── README.md
│   └── TASK_1.3_TEST_GUIDE.md
└── TestFiles/                 # 测试相关文件
```

---

## 🚀 新手快速开始

### **打不开 PropertyDumper？想立即测试？**

👉 **立即查看** [`QUICK_START.md`](QUICK_START.md) - 5分钟完成测试！

---

## 🎯 工作流程

### 1. **开始工作前**
查看 `TASK_ROADMAP.md`，了解：
- 当前进度（已完成/进行中的任务）
- 下一个任务的目标和要求
- 预期结果和测试方法

### 2. **开发过程中**
查阅 `TECHNICAL_NOTES.md`，获取：
- API 使用方法
- 代码示例
- 关键类和方法说明
- 常见问题解决方案

记录新的技术发现到 `TECHNICAL_NOTES.md`

### 3. **完成任务后**
更新 `TASK_ROADMAP.md`：
- ✅ 标记任务完成
- 📝 记录完成日期
- 💡 添加重要发现或改进建议
- 🔄 必要时调整后续任务规划

---

## 📊 当前状态

**进度**: 10% (2/21 任务完成)

**已完成**:
- ✅ Task 1.1 - 理解 BP2AI 架构
- ✅ Task 1.2 - 分析 BP2AI 入口点

**进行中**:
- 🔄 Task 1.3 - 编写自动化导出测试函数（代码完成，等待测试）

**下一步**:
- Task 2.1 - 研究如何获取蓝图的所有图表

---

## 💡 文档维护原则

1. **精简至上**: 只维护核心文档，避免重复
2. **及时更新**: 完成工作后立即更新文档
3. **技术优先**: 重要技术细节必须记录在 TECHNICAL_NOTES.md
4. **进度透明**: TASK_ROADMAP.md 反映真实进度

---

## 🗂️ 归档说明

`archive/` 文件夹包含项目早期创建的详细文档，这些文档完成了历史使命：
- 帮助理解项目背景
- 记录了从 VSCode 到 Rider 的迁移过程
- 提供了详细的入门指南

现在项目已进入稳定开发阶段，我们精简为两个核心文档，保持工作区整洁。

---

**准备好了？** 👉 开始查看 [`TASK_ROADMAP.md`](TASK_ROADMAP.md)

