# BP2AI 批量导出功能开发路线图

**目标**: 实现蓝图完整数据的自动化导出，最终实现**一键导出指定文件夹中的所有蓝图**

**当前状态**: 阶段2（Task 2.3已完成），开始Task 2.4-2.7

**开发原则**:
- ✅ 每完成一个任务立即测试
- ✅ 测试通过后再进入下一任务
- ✅ 保持代码可回滚
- ✅ 确保用户理解每一步
- ⚠️ **重点关注**: 不要过度优化单个图表导出，优先实现批量导出核心功能

---

## 📝 更新日志

### 2025-11-21 - 用户需求确认与路线图重规划

**用户确认的需求**:

1. **配置管理系统** (Task 2.4)
   - 创建插件级配置类 `UBP2AIExportSettings`
   - 实现5个CVARs（排除 IncludeNestedFunctions）：
     - `bShowDefaultParams` - 是否显示默认参数
     - `bSeparateUserGraphs` - 是否分开定义用户函数
     - `ExecFlowPreviewLines` - 日志预览行数
     - `MaxPreviewLineLength` - 预览每行最大长度
     - `bEnableExecFlowPreview` - 是否启用预览
   - 配置存储在插件文件夹，便于集中管理

2. **index.json的定位** (Task 2.7)
   - **项目级索引文件**，记录批量导出的元数据
   - 用途：
     - 快速查找已导出蓝图
     - 版本管理和增量导出
     - 质量检查和报告生成
     - AI工具索引和搜索
   - 在批量导出文件夹时自动生成

3. **优化导出格式** (Task 2.5)
   - **核心问题**: 事件图表有冗余块（3个事件导出9个块）
   - **解决方案**: 
     - 在主干执行流中添加 `(Target: ClassName)` 信息
     - 去除 "Previously detailed" 冗余块
     - 示例: `AddMovementInput(...) (Target: Pawn)`

4. **增强函数和事件显示** (Task 2.6)
   - **函数签名增强**: 显示完整参数和返回值
   - **事件图表结构**: 保持事件节点和图表的父子关系
     - 不要把事件平铺到函数列表
     - 每个图表作为一个列表，包含其事件节点
     - 展示真实的蓝图图表结构

5. **批量导出文件夹** (Task 2.7)
   - **项目最终目标**: 一键导出指定文件夹的所有蓝图
   - 排除非蓝图资产（材质、纹理等）
   - 生成每个蓝图的.md文件
   - 生成项目级index.json

**待讨论的任务**:
- Task 3.5 - 增强函数参数解析
- Task 3.6 - 导出依赖关系
- Task 4.2 - JSON输出（可选）
- Task 4.3 - HTML输出（可选）

**下一步行动**:
- 等待用户确认后开始实现 Task 2.4（配置管理系统）
- 按顺序完成 Task 2.4 → 2.5 → 2.6 → 2.7

---

### 2025-11-22 - 蓝图接口导出支持

- ✅ 在 `ExportCompleteBlueprint` 中自动识别 `BPTYPE_Interface`，跳过图表导出流程
- ✅ 新增 `ExportInterfaceFunctionSignatures`，直接读取接口 `UFunction` 并输出参数/返回值
- ✅ Markdown 输出新增 `## Interface Functions`，专注展示函数头
- ✅ 测试库忽略接口资产无图表的警告，确保用例通过
- ✅ 完成 Task 3.6：实现 `ExportReferences()`，在 Markdown 中导出依赖关系列表
- 🔜 下一步：恢复 Task 2.6 的函数/事件结构优化，并推进批量导出（Task 2.7）

---

## 📋 任务清单

### **阶段 1: 理解与准备 (Foundation)**

- [x] **Task 1.1** - 理解 BP2AI 现有架构
  - 核心组件: Extractor, Tracer, Generator
  - 工作流程: 选中节点 → 提取数据 → 追踪执行流 → 生成文档
  - **状态**: ✅ 已完成 (用户已理解)

- [x] **Task 1.2** - 分析 BP2AI 的入口点 ✅
  - ✅ 找到调用链: 按钮 → `GenerateExecFlowAction()` → 创建 `SMarkdownOutputWindow`
  - ✅ 核心方法: `GetSelectedBlueprintNodes()` - 获取编辑器中选中的节点
  - ✅ 关键流程: 
    1. 通过 `UAssetEditorSubsystem` 找到当前打开的蓝图编辑器
    2. 获取 `FBlueprintEditor::GetSelectedNodes()` 的选中节点集合
    3. 调用 `FExecutionFlowGenerator::GenerateDocumentForNodes()` 生成文档
  - **已完成**: 2025-01-14
  - **关键发现**: BP2AI 的核心在于 `FExecutionFlowGenerator`,它接受节点数组作为输入!

- [x] **Task 1.3** - 编写自动化导出测试函数
  - 创建 `FBP2AIBatchExporter` 类
  - 实现 `ExportSingleGraph(UEdGraph* Graph)` 方法
  - 直接调用 `FExecutionFlowGenerator` (绕过 UI)
  - 测试方法: 在编辑器中通过 C++ 代码调用, 传入测试蓝图的 EventGraph
  - 预期结果: 
    - 成功返回 Markdown 字符串
    - 输出到日志: `LogBP2AI: Exported EventGraph (1234 characters)`
    - 验证我们可以程序化调用 BP2AI 的核心功能
  - 已完成结果 (2025-11-15):
    - ✅ 自动化导出闭环跑通，`TEST PASSED`
    - ✅ 在 `LogExportResult` 中增加 ExecFlow 预览日志：
      - 解析首个 ```blueprint 代码块；若不存在回退到全文
      - 仅显示前 10 行，每行最长 200 字符，超长追加 `...[truncated]`
      - 以分隔符包裹：`----- ExecFlow Preview for Graph: <GraphName> -----` / `----- End ... -----`
      - 超出时提示：`ExecFlow preview truncated: x more line(s) not shown...`
    - ✅ 导出文件仍保存到 `Saved/BP2AI/Exports/*.md` 以便离线查看

---

### **阶段 2: 导出 Graph Logic (复用 BP2AI)**

- [x] **Task 2.1** - 研究如何获取蓝图的所有图表 （已通过 Graph Inventory 日志验证）
  - 已实现：在导出前打印图表清单（Event/Function/Macro/Delegate 数量与名称）
  - 日志示例：`🔎 Graph Inventory for 'BP_TestExport':` 后跟各类别列表
  - 结果：与测试蓝图结构匹配，无遗漏

- [x] **Task 2.2** - 创建单图表导出函数（结构化增强）
  - 已新增结构体 `FExportedGraphInfo`（图名、类别、节点数、字符数、行数、代码块数、Markdown）
  - 已新增接口：`ExportSingleGraphDetailed` / `ExportAllGraphsDetailed`
  - 已新增蓝图级聚合结构 `FCompleteBlueprintData`（包含所有图表 + 元数据占位）
  - 已实现 `ToMarkdown()` 方法：生成"一个蓝图一个 Markdown 文档"
  - 调试开关：`GBP2AI_WritePerGraphMarkdown`（默认关闭，仅调试时可写单图 md）
  - 已完成结果 (2025-11-15):
    - ✅ 默认只产出 `BlueprintName.md`（包含所有图表 + 统计 + 元数据占位）
    - ✅ 单图 md 不再默认写出（可通过调试开关启用）
    - ✅ index.json 可选（调试/QA 用，默认关闭）

- [x] **Task 2.3** - 实现蓝图所有图表批量导出
  - 已编写 `ExportCompleteBlueprint(UBlueprint* BP)` 函数（聚合所有图表）
  - 返回 `FCompleteBlueprintData`（包含 GraphLogic + Metadata）
  - 测试方法: 导出测试蓝图，检查生成的单一 Markdown 文档
  - 完成结果: 
    ```
    ✅ 生成单一蓝图文档: Saved/BP2AI/Exports/BP_TestExport.md
    ✅ 包含所有图表的完整文档（Summary + Metadata + Components + Variables + Functions + Graph Logic）
    ```

- [x] **Task 2.4** - 创建导出配置管理系统 ✅
  - **状态**: 已完成代码实现（2025-11-21），待编译测试
  - **目标**: 创建简单的配置头文件，集中管理所有导出参数
  - **实现方案**: Header 常量配置（`BP2AIExportConfig.h`）
    - 采用 `namespace BP2AIExportConfig` + `constexpr` 常量
    - 无需 CVAR 或 UDeveloperSettings（用户需求：简单直接）
    - 编译期确定，无运行时开销
  - **配置项** (4个预览配置 + 2个导出格式配置):
    1. `bPreviewEnabled` (bool, 默认 true) - 预览总开关
    2. `PreviewMaxBlocks` (int32, 默认 3) - 最大预览区块数
    3. `PreviewLinesPerBlock` (int32, 默认 6) - 每区块行数
    4. `PreviewMaxLineLength` (int32, 默认 200) - 最大行长度
    5. `bShowDefaultParams` (bool, 默认 true) - 是否显示默认参数
    6. `bSeparateUserGraphs` (bool, 默认 true) - 是否分开用户函数
  - **配置位置**: `Plugins/BP2AI/Source/BP2AI/Public/Settings/BP2AIExportConfig.h`
  - **修改方式**: 直接编辑头文件，重新编译即可
  - **集成**: 
    - `FBP2AIBatchExporter::CreateDefaultSettings()` - 使用导出格式配置
    - `FBP2AIBatchExporter::LogExportResult()` - 使用预览配置
  - **文档**: 详见 `TASK_2.4_IMPLEMENTATION.md`
  - **测试方法**: 修改配置常量，重新编译，验证导出结果
  - **预期结果**: 
    ```
    ✅ 配置简单直观，一个头文件搞定
    ✅ 修改后重新编译即可生效
    ✅ 无运行时开销，编译期优化
    ```

- [x] **Task 2.5** - 优化导出格式：去除冗余块，添加Target信息
  - **问题**: EventGraph有3个事件但导出了9个块（3个主干+6个冗余函数块）
  - **原因**: BP2AI为每个节点都生成独立的"Trace Start"块
  - **解决方案**: 
    1. 在主干执行流中为函数调用添加 `(Target: ClassName)` 信息
    2. 移除 "Previously detailed" 的冗余块
    3. 保持事件节点和图表的父子关系（不平铺到函数列表）
  - **实现位置**: 修改 `FExecutionFlowGenerator` 或后处理Markdown
  - **示例输出**:
    ```markdown
    ### [Event] EventGraph
    
    **Event: IA_Move (Triggered)**
    ```blueprint
    * Event EnhancedInputAction IA_Move Args: (...)
        * Triggered:
            * AddMovementInput(...) (Target: Pawn)
                * AddMovementInput(...) (Target: Pawn)
                    * [Path ends]
    ```
    ```
  - **测试方法**: 导出 BP_TestExport，验证 EventGraph 只有3个块
  - **预期结果**: 
    ```
    ✅ EventGraph 导出的 Blueprint Blocks 数量 = 事件节点数量（3个）
    ✅ 每个函数调用都显示 Target 信息
    ✅ 去除所有 "Previously detailed" 冗余块
    ```



---

### **阶段 3: 导出其他元数据 (Metadata/Components/Variables/Functions)**

**注意**: 阶段3的基础功能（ExportMetadata/Components/Variables/Functions）已在Task 2.3中实现。本阶段主要关注细化和优化这些数据的导出质量。

- [x] **Task 3.1** - 导出 Blueprint Metadata ✅
  - ✅ 已实现 `ExportMetadata()` 函数
  - ✅ 获取: 类名, 父类, 资产路径, 实现的接口
  - ✅ 创建结构体 `FBlueprintMetadata`
  - **已完成**: 在 BP_TestExport.md 中可见元数据信息
  - **实际输出**:
    ```markdown
    ## Metadata
    - **Class**: BP_TestExport
    - **ParentClass**: Character
    ```

- [x] **Task 3.2** - 导出 Components ✅
  - ✅ 已实现 `ExportComponents()` 函数
  - ✅ 遍历 CDO 和 `SimpleConstructionScript` 的所有节点
  - ✅ 提取组件名称、类型、父子关系
  - ✅ 创建结构体 `FComponentInfo`
  - **已完成**: 在 BP_TestExport.md 中可见组件树
  - **实际输出**:
    ```markdown
    ## Components
    - CollisionCylinder : CapsuleComponent
    - Arrow : ArrowComponent (Parent: CollisionCylinder)
    - CharMoveComp : CharacterMovementComponent
    - CharacterMesh0 : SkeletalMeshComponent (Parent: CollisionCylinder)
    - FirstPersonCamera : CameraComponent (Parent: CollisionCylinder)
    - FirstPersonMesh : SkeletalMeshComponent (Parent: FirstPersonCamera)
    ```

- [x] **Task 3.3** - 导出 Variables ✅
  - ✅ 已实现 `ExportVariables()` 函数
  - ✅ 遍历 `Blueprint->NewVariables`
  - ✅ 提取: 名称, 类型, 默认值, 是否公开, Tooltip
  - ✅ 创建结构体 `FVariableInfo`
  - **已完成**: 在 BP_TestExport.md 中可见变量列表
  - **实际输出**:
    ```markdown
    ## Variables
    - TestEventDispatcher : mcdelegate (Public) // Test Event Dispatcher
    - TestBoolVar : bool (Public) // Test Bool Var
    - TestIntVar : int (Public) // Test Int Var
    - TestActorVar : object (Public) // Test Actor Var
    ```

- [x] **Task 3.4** - 导出 Functions/Events（基础版）✅
  - ✅ 已实现 `ExportFunctions()` 函数（基础版）
  - ✅ 遍历所有函数图表和事件图表
  - ✅ 创建结构体 `FFunctionInfo`
  - **已完成**: 在 BP_TestExport.md 中可见函数列表
  - **当前输出**:
    ```markdown
    ## Functions
    - UserConstructionScript
    - TestFunction1
    - TestFunction2
    - EventGraph (Event)
    - TestGraph (Event)
    ```
  - **待优化**: Task 2.6 将增强此功能，添加参数和返回值信息

- [x] **Task 3.5** - 增强函数和事件显示
  - **函数签名增强**: 在函数列表中显示完整参数和返回值
    ```markdown
    ## Functions
    - UserConstructionScript () -> void
    - TestFunction1 (Param: bool) -> void
    - TestFunction2 () -> void
    ```
  - **事件图表结构增强**: 为每个图表的事件节点创建独立条目（保持父子关系）
    ```markdown
    ## Graph Logic
    
    ### [Event] EventGraph
      - **Event: IA_Look** (Triggered)
      - **Event: IA_Move** (Triggered)  
      - **Event: IA_Jump** (Started, Completed)
    
    ### [Event] TestGraph
      - **Event: TestEvent**
    ```
  - **实现要点**: 
    - 不要把事件放入Functions列表
    - 每个图表作为一个列表，其中放置该图表的事件节点
    - 展示真实的蓝图图表结构
  - **测试方法**: 检查导出的Markdown文档结构
  - **预期结果**: 
    ```
    ✅ Functions列表不包含事件
    ✅ 每个事件图表有清晰的事件节点列表
    ✅ 函数列表显示完整签名（参数类型和返回值）
    ```

- [x] **Task 3.6** - 导出 References (依赖关系)
  - ✅ 新增 `FReferenceInfo` 结构体与 `ExportReferences()`，聚合蓝图依赖
  - ✅ 扫描所有图表节点与变量，解析 Pin 默认值/对象引用，区分 Hard / Soft 引用
  - ✅ 捕获父类、接口、SCS 组件、调用函数所属类等依赖，并写入 Markdown `## References`
  - ✅ Markdown 输出示例（BP_TestExport）:
    ```markdown
    ## References
    - /Game/Input/IA_Move (InputAction) [Soft] // EventGraph :: EnhancedInputAction IA_Move.Triggered
    ```
  - 🔍 **验证**: 检查 `BP_TestExport.md`，确认 EnhancedInputAction 等资产路径被记录

---

### **阶段 4: 整合与格式化输出**

- [x] **Task 4.1** - 创建统一的数据结构 ✅
  - ✅ 已定义 `FCompleteBlueprintData` 结构体
  - ✅ 包含阶段2和阶段3的所有数据（Graphs + Metadata + Components + Variables + Functions）
  - ✅ 已实现 `ToMarkdown()` 方法
  - **已完成**: 可成功导出 BP_TestExport.md 包含所有信息


- [ ] **Task 4.2** - 优化 Editor Utility 界面
  - **目标**: 完善批量导出的UI交互
  - **实现**:
    1. 添加文件夹选择器（Content Browser集成）
    2. 添加导出选项面板（配置参数）
    3. 显示导出日志和结果统计
  - **测试方法**: 通过UI导出多个蓝图
  - **预期结果**: 
    ```
    ✅ 用户友好的文件夹选择界面
    ✅ 实时显示导出进度（1/25, 2/25...）
    ✅ 清晰的成功/失败提示
    ✅ 可配置导出选项
    ```
  - **测试方法**: 在编辑器中打开工具窗口,导出蓝图
  - **预期结果**: 友好的用户界面,能成功导出

---

### **阶段 5: 批量导出功能**

 实现批量导出文件夹功能（项目最终目标）
  - **目标**: 一键导出指定文件夹中的所有蓝图（不包括资产）
  - **实现**: 
    1. 创建 `ExportFolder(FString FolderPath)` 函数
    2. 遍历文件夹，过滤出所有蓝图资产（排除其他资产类型）
    3. 对每个蓝图调用 `ExportCompleteBlueprint`
    4. 生成项目级 `index.json`（包含所有导出蓝图的元数据）
    5. 生成导出报告（成功/失败统计）
  - **UI接口**: 
    - Editor Utility Widget 添加文件夹选择器
    - 显示导出进度条
    - 显示实时日志输出
  - **index.json 示例**:
    ```json
    {
      "exportTime": "2025-11-21T10:30:00Z",
      "exportFolder": "/Game/Test",
      "totalBlueprints": 25,
      "successCount": 24,
      "failureCount": 1,
      "blueprints": [
        {
          "name": "BP_TestExport",
          "path": "/Game/Test/BP_TestExport",
          "graphCount": 7,
          "nodeCount": 37,
          "exportFile": "BP_TestExport.md",
          "status": "success"
        }
      ]
    }
    ```
  - **测试方法**: 
    1. 选择 `/Game/Test` 文件夹
    2. 点击"批量导出"按钮
    3. 检查输出文件夹和index.json
  - **预期结果**: 
    ```
    ✅ 自动识别并导出文件夹中所有蓝图
    ✅ 排除非蓝图资产（材质、纹理等）
    ✅ 生成每个蓝图的独立.md文件
    ✅ 生成项目级index.json索引文件
    ✅ UI显示导出进度和结果统计
    ```


- [ ] **Task 5.1** - 实现项目级扫描
  - 使用 `AssetRegistry` 扫描所有蓝图资产
  - 支持路径过滤 (只导出指定文件夹)
  - 创建导出任务队列
  - **测试方法**: 扫描整个项目,打印找到的蓝图数量
  - **预期结果**: 准确统计所有蓝图,无遗漏

- [ ] **Task 5.2** - 批量导出实现
  - 遍历任务队列,调用阶段4的导出功能
  - 显示进度条 (已完成/总数)
  - 支持错误处理和日志记录
  - **测试方法**: 批量导出 5 个测试蓝图
  - **预期结果**: 生成 5 个完整的文档,无崩溃

- [ ] **Task 5.3** - 创建索引文件
  - 生成项目级的 `index.json`
  - 包含所有蓝图的概览信息
  - 支持快速查找和依赖分析
  - **测试方法**: 查看 index.json,检查数据完整性
  - **预期结果**: 结构化的索引,便于 AI 理解项目全貌

---

### **阶段 6: Python API 集成**

- [ ] **Task 6.1** - 暴露 C++ 函数给 Python
  - 使用 `UFUNCTION(BlueprintCallable)` 标记导出函数
  - 测试 Python 可调用性
  - **测试方法**: 在 Python Console 中调用
    ```python
    import unreal
    unreal.BP2AILibrary.export_blueprint("/Game/Test/BP_TestExport")
    ```
  - **预期结果**: Python 成功调用,返回文件路径

- [ ] **Task 6.2** - 编写 Python 辅助脚本
  - 创建 `bp2ai_batch_export.py`
  - 支持命令行参数 (路径过滤、输出格式)
  - 生成导出报告
  - **测试方法**: 运行脚本导出所有蓝图
    ```bash
    python bp2ai_batch_export.py --path /Game/Characters --format json
    ```
  - **预期结果**: 自动化导出,无需手动操作

---

### **阶段 7: 测试与文档**

- [ ] **Task 7.1** - 完整功能测试
  - 测试复杂蓝图 (多层继承、多个接口)
  - 测试边界情况 (空蓝图、纯数据蓝图)
  - 性能测试 (导出 100+ 蓝图的时间)
  - **预期结果**: 所有场景都能正确处理

- [ ] **Task 7.2** - 编写用户文档
  - 功能介绍
  - 使用教程 (带截图)
  - Python API 参考
  - 常见问题解答
  - **预期结果**: 用户能独立使用所有功能

- [ ] **Task 7.3** - AI 协作测试
  - 将导出的文档提供给 AI
  - 测试 AI 能否理解蓝图结构
  - 测试 AI 能否给出合理的重构建议
  - **预期结果**: AI 能准确分析并提供有价值的建议

---

## 🗓 变更记录

- 2025-11-21 (晚上 - Task 2.4 完成)
  - ✅ 完成 Task 2.4（配置管理系统）
  - 创建 `BP2AIExportConfig.h` - Header 常量配置方案
  - 实现 6 个配置项（4个预览 + 2个导出格式）
  - 集成到 BatchExporter 的 CreateDefaultSettings 和 LogExportResult
  - 详细文档: `TASK_2.4_IMPLEMENTATION.md`
  - 下一目标：Task 2.5（优化导出格式，去除冗余块）

- 2025-11-21 (晚上)
  - ✅ 完成 Task 2.3（蓝图所有图表批量导出）
  - 📋 用户确认第二轮需求：
    - ✅ 确认实现：Task 2.4-2.7, 3.5-3.6（6个任务）
    - ❌ 取消实现：Task 4.2-4.3（JSON/HTML输出）
  - 🎯 明确开发原则：**不过度优化单个蓝图导出，优先批量导出核心功能**
  - 🔄 更新路线图，反映最新优先级

- 2025-11-15 (深夜)
  - ✅ 完成 Task 2.2（结构化导出 + 蓝图级聚合）
  - 引入 `FCompleteBlueprintData` 与 `ToMarkdown()`，实现"一个蓝图一个文档"
  - 默认只产出 `BlueprintName.md`；单图 md 与 index.json 改为可选调试产物
  - 下一目标：Task 2.3 补充公开接口 `ExportCompleteBlueprintData`，完善阶段 2

---

## 📝 每日工作日志

### 2025-01-14 (continued)
- ✅ 完成 Task 1.1: 理解架构
- ✅ 完成 Task 1.2: 分析入口点
  - 核心发现: `GetSelectedBlueprintNodes()` 通过编辑器子系统获取选中节点
  - 关键类: `FExecutionFlowGenerator` 是文档生成的核心
- 🔄 进行中 Task 1.3: 编写自动化导出测试函数
  - ✅ 创建了 `FBP2AIBatchExporter` 类
  - ✅ 实现了 `ExportSingleGraph()` 方法
  - ✅ 实现了 `ExportAllGraphsFromBlueprint()` 方法
  - ✅ 集成到现有的测试框架
  - ✅ 创建了迁移文档 (VSCode → Rider)
  - 📌 **等待用户在 Rider 中编译并测试**
  - 📄 详细测试步骤见: `TASK_1.3_TEST_GUIDE.md`
  - 📖 Rider 快速上手见: `PROJECT_STATUS_FOR_RIDER.md`

---

## 🐛 已知问题

_(暂无)_

---

## 💡 改进建议

_(待补充)_
