# Stage Set Exporter（UE 5.7）

Editor Utility Widget 导出工具。收集当前编辑器场景中带 StageSetExportComponent 的 Actor，按类型/Tag/选择/当前 Level 筛选，将世界坐标和自定义整数字段导出至 UStageSetTable。

本目录是可复制到项目的 C++ 插件源码，未预制 .uasset 蓝图资源。已在独立验证工程中使用本机 UE 5.7.4 完成 UHT、Development Editor / Win64 编译及两项无界面编辑器自动化测试；详细记录见 [UE57Validation.md](UE57Validation.md)。尚未对实际项目的交互界面和打包版本做验证。

## 安装

1. 关闭 UE 编辑器。
2. 将整个 `StageSetExporter` 文件夹复制到实际工程的 `Plugins/StageSetExporter` 中。
3. 用 Rider 重新加载工程并构建 Development Editor / Win64。
4. 打开 UE，在 Plugins 中确认 Stage Set Exporter 已启用。Editor Utility Widget 使用引擎的 Blutility；若项目里看不到 Editor Utilities 相关菜单，检查 Editor Scripting Utilities 是否启用并重启。

插件包含两个模块：

- StageSetRuntime：UStageSetTable、FStageSetRow、UStageSetExportComponent。表和行结构可在游戏中读取，没有 UnrealEd 依赖。元数据组件本身标记 EditorOnly，Cook 时剔除。
- StageSetEditor：UEditorUtilityWidget、资产工厂、编辑器查询与保存逻辑，仅供编辑器目标使用。

若游戏模块需要在 C++ 中引用表/行结构，在游戏模块 Build.cs 的依赖中添加 `StageSetRuntime`。纯蓝图使用不需要修改游戏模块依赖。

## 数据结构

UStageSetTable 继承 UDataTable，构造函数固定 RowStruct 为 FStageSetRow。

| 字段 | 类型 | 来源 |
| --- | --- | --- |
| RowName | FName（表的行键） | Actor 上组件的 RowName，例如 Actor_001 |
| Category | int32 | 组件的 Category |
| Id | int32 | 组件的 Id |
| AreaId | int32 | 组件的 AreaId |
| Transform | FTransform | Actor.GetActorTransform()，包含世界位置、旋转、缩放 |
| Param01 | int32 | 组件的 Param01 |
| Param02 | int32 | 组件的 Param02 |
| Param03 | int32 | 组件的 Param03 |
| Param04 | int32 | 组件的 Param04 |

UE DataTable 本身带唯一 RowName，不能把表字段直接声明在 UDataTable 子类里当作每行数据。因此不在 FStageSetRow 中重复定义另一个 RowName；编辑器表格首列就是你需要的唯一标识。

AreaId 为手动填写的关卡区域编号，不自动推断 Sublevel。整数 Id 不根据 RowName 自动生成，也不强制全表唯一（你可以自行规定 Category/AreaId 内的 ID 规则）。

## 1. 配置场景 Actor

以现有 BP_CheckpointActor 为例：

1. 打开 Actor 蓝图，Add Component → Stage Set Export。
2. 该组件只保存导出元数据，不影响原 Actor 的文字、碰撞或游戏行为。
3. 将 BP 放入关卡，在每个实例的组件 Details → Stage Set 中填写 RowName、Category、Id、AreaId、Param01～04。
4. RowName 应在同一张目标表的所有来源中唯一。不要在蓝图类默认值中填写所有实例共用的 RowName；复制 Actor 后也应重新分配名称。
5. 原生场景 Actor 也可以通过 Details 的 Add Component 添加此组件。每个 Actor 只添加一个。

工具只导出带该组件的 Actor，没有组件的灯光、相机等会被跳过。Transform 始终来自 Actor 本身，组件没有 Transform 字段。

## 2. 创建目标表

在 Content Browser 右键 → Miscellaneous 中找到 Stage Set Table，创建 `DT_StageSet` 并保存。插件提供 UStageSetTableFactory，创建出的资产确实为 UStageSetTable 子类，并已设置行结构；不要用普通 Data Table 资产代替。

若当前编辑器未显示新建入口，先确认插件 Editor 模块编译、加载成功并重启。支持 Python 的项目也可在编辑器 Python 控制台创建一次（替换目标目录，目录需已存在）：

```python
import unreal
unreal.AssetToolsHelpers.get_asset_tools().create_asset(
    "DT_StageSet", "/Game/Data", unreal.StageSetTable, unreal.StageSetTableFactory())
```

工具的 Target Table 选择器仅接受 UStageSetTable。导出时再次验证 Row Structure 为 FStageSetRow。

## 3. 创建 Editor Utility Widget 蓝图

1. Content Browser → Editor Utilities → Editor Utility Widget，创建 `EUW_StageSetExporter`。
   - UE 5.7 的创建窗口标题通常是 Pick Root Widget for New Editor Utility Widget，它只列出 Panel 根控件，不是父类列表，因此不会显示 StageSetExportWidget。这里可选择 Canvas Panel 或 None。
2. 打开它，通过 File → Reparent Blueprint（或 Class Settings 中的父类设置）将父类改为 **StageSetExportWidget**。
3. Compile 并 Save。无须创建 Designer 控件、Event Graph 或按钮绑定；父类 RebuildWidget 已生成原生界面，Designer 中添加的控件不会用于此界面。
4. 在 Content Browser 右键该 Widget，选择 **Run Editor Utility Widget**。

打开后的面板包含设置区域、自动命名、验证、导出、保存按钮和状态信息。工具不能在 PIE / Simulate 中执行导出，请先停止游戏。

## 4. 使用工具

1. Target Table：选择 DT_StageSet。
2. Actor Class：选择 CheckpointActor 或其 BP 类；会包含继承类。留空或 AActor 表示不限类型，但仍必须带导出组件。
3. Required Actor Tag：可选，仅匹配 Actor Tags，不是组件 Tags。
4. Only Selected Actors：开启后仅导出当前选中的匹配 Actor。
5. Only Current Level：开启后只导出 Levels 面板当前编辑 Level 中的匹配 Actor；关闭时包含编辑器已加载的其他 Level。
6. Export Mode：
   - Upsert（默认）：更新相同 RowName、添加新行，保留表中其他行。
   - Replace All：清空表，再写入当前筛选结果；只适合该筛选范围专用的整张表。选中对象导出时禁止 Replace All。
7. Save After Export：默认开启，成功导出后保存目标表；关闭时只修改内存并标记 Dirty，可先查看表，再点击 Save Target Table。
8. 可先点 Assign missing RowNames，自动为缺名 Actor 分配 Actor_001、Actor_002 等名字。会避开所有已加载 Actor 及目标表已使用的名称；已有名称不改动，整数 Id 不改动。随后保存场景（包括外部 Actor 文件）。
9. 点击 Validate Actors。通过后点击 Export Actors to Table。

所有操作会把结果和错误输出到面板及 Output Log（LogStageSetExporter）。缺名、重复名称、多个导出组件或非法 Transform 时，在修改表之前终止。自动命名只填补空值，不自动修复重名。重复名称检查也会检查当前筛选之外的已加载导出 Actor。

导出和自动命名支持编辑器 Undo。若表已经保存到磁盘，Undo 后需要再次保存才能让磁盘同步回退。自动命名不会自动保存关卡。

## 导出范围与数据维护

- 只收集当前 Editor World 已加载的 Actor。未加载 Sublevel、World Partition 未加载区域、其他未打开地图不会被自动加载，也不会被导出。
- Level Instance 内容是否包含在查询中取决于它在当前编辑器世界中的加载/展开状态；本工具不扫描未加载的关卡资产。
- Replace All 前请加载完整来源范围；工具无法知道还有哪些未加载的 Actor，不能保证自动发现缺失来源。
- 没有匹配 Actor 时拒绝写表，即使 Replace All 也不会把目标清空。
- Upsert 不删除旧行。删除场景 Actor 后，需要手动删表行，或在加载完整范围后使用 Replace All。
- RowName 一旦分配就保存在 Actor 组件上；它不会随着遍历顺序或 Actor Label 变化而重新编号。但没有加载的其他地图无法参与重名检测，跨地图共用表时建议使用区域前缀，例如 Area01_Actor_001。
- Transform 是世界空间。若以后要把数据放到不同位置的 Level Instance 中复用，需要另加相对坐标导出模式。
- SaveLoadedAsset 保存失败时，已生成的数据仍在内存中，状态栏会明确报告保存失败；请解决只读/版本控制等问题后重新保存。
- 仅供生成开发期数据，不作为打包游戏中的存档工具。运行时可正常读取生成的 UStageSetTable。

## 接入验证

已通过：UHT / C++ / DLL 链接、以原生类为父类创建并编译 EUW、原生 Slate 界面构建、两行数据导出（全部字段及世界 Transform）、Upsert 更新、重复键拒绝、空范围 ReplaceAll 保护。以下仍建议在你的实际项目中完成：

1. 创建两个不同 Transform 的 Actor，填写不同 RowName 和参数；导出后检查每列及旋转/缩放。
2. 重复导出同名行，确认更新而非重复追加。
3. 制造重复 RowName，确认导出失败且原表不变。
4. 验证类型/Tag/选中/当前 Level 筛选。
5. 验证 Upsert 保留其他行，Replace All 在完整范围内删除旧行。
6. 关闭再打开编辑器，确认表和自动生成的 RowName 都已保存。
7. 构建游戏目标，确认不依赖 StageSetEditor，表可读取而导出组件被剔除。
