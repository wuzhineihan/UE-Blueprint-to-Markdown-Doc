# Unreal 引擎蓝图转 Markdown 导出器

这是一个 Unreal Engine 5 插件，旨在将蓝图逻辑和结构提取为人类可读且对 AI 友好的 Markdown 文档。它有助于记录复杂的蓝图系统，并为 AI 编程助手提供上下文。

## 功能

*   **蓝图转 Markdown**: 将蓝图图表、函数、变量和逻辑转换为结构化的 Markdown。
*   **执行流分析**: 分析并可视化蓝图图表的执行流程。
*   **批量导出**: 从内容浏览器中导出多个蓝图或整个文件夹。
*   **广泛支持**: 支持标准蓝图、蓝图接口和控件蓝图。
*   **可配置**: 通过 C++ 配置文件自定义导出细节和格式。

## 安装

1.  将 `BP2AI` 文件夹复制到您项目的 `Plugins` 目录中（例如 `YourProject/Plugins/BP2AI`）。
2.  重新生成项目文件。
3.  构建您的项目。
4.  在 Unreal 编辑器中启用插件 (Edit > Plugins > BP2AI)。

## 使用方法

### 1. 蓝图编辑器工具栏
在编辑器中打开蓝图后，点击工具栏中的 **BP2AI** 按钮。
*   这将打开 **Blueprint Execution Flow** (蓝图执行流) 窗口。
*   它会分析当前蓝图，并以 Markdown 友好格式显示其执行流和逻辑。

### 2. 内容浏览器 (单个资产)
在内容浏览器中右键点击任何蓝图资产。
*   从上下文菜单中选择 **Export Blueprint (BP2AI)**。
*   插件将在资产旁边或指定的导出文件夹中生成一个 Markdown 文件。

### 3. 内容浏览器 (批量导出)
在内容浏览器中右键点击任何文件夹。
*   从上下文菜单中选择 **Export Blueprints (BP2AI)** (或类似的选项)。
*   这将递归地查找文件夹中的所有蓝图并将它们导出为 Markdown。

## 配置

配置当前通过 C++ 头文件进行。要更改设置，请修改 `Source/BP2AI/Public/Settings/BP2AIExportConfig.h` 并重新编译插件。

**可用设置:**

*   `bPreviewEnabled`: 启用/禁用执行流日志预览。
*   `PreviewMaxBlocks`: 限制预览中显示的块数。
*   `bShowDefaultParams`: 在函数调用中显示默认参数值。
*   `bSeparateUserGraphs`: 将用户定义的图表与主事件图表分开。
*   `bDetailedBlueprintLog`: 启用详细日志以进行调试。
*   `bSilenceInternalCategoriesDuringBatchExport`: 在批量操作期间静默内部日志，以提高性能并减少噪音。

## 输出格式

插件生成的 Markdown 文件包含：
*   类层级和接口。
*   变量和属性。
*   函数及其签名。
*   事件图和执行流逻辑。

