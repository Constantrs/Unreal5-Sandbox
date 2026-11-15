# SideScroller：2.5D 横板原型

此后的 2.5D 相关源码集中在工作目录 SideScroller 中。本文件夹为独立的 UE 5.7 插件，不依赖工作目录原有 Source 或 StageSetExporter。

已使用本机 UE 5.7.4 完成 UHT、Development Editor / Win64 编译和分页/数据绑定自动化测试。还通过 UE 命令行生成了真实的示例 DataTable：`Content/Data/DT_StageInfo.uasset`。

## 安装与编译

关闭 UE，将此文件夹复制为实际工程的 `Plugins/SideScroller`，用 Rider 构建工程的 Development Editor / Win64，重启 UE 并确认 Side Scroller Prototype 插件已启用。复制时不需要 Validation 目录以及 Binaries/Intermediate/Saved 等生成目录。

Runtime 模块负责数据、UMG、输入和 OpenLevel；Editor 模块只负责 DataTable 创建工厂和测试。UMG 界面由 C++ 生成，不需要自行制作 Widget Blueprint；若派生 Widget Blueprint，保持其父类 SideScrollerStageSelectWidget，界面仍使用父类构建的布局。

## 关卡数据表

USideScrollerStageTable 继承 UDataTable，固定行结构 FSideScrollerStageRow：

| 字段 | 类型 | 用途 |
| --- | --- | --- |
| StageName | FName | 界面显示的关卡名称 |
| LevelName | FString | OpenLevel 使用的实际关卡名，推荐完整包路径 |

DataTable 自带 RowName，作为行键。菜单按 RowName 字典序排序；建议命名 Stage001、Stage002…，不依赖数据表内部的遍历顺序。

可以直接使用插件已生成的 `/SideScroller/Data/DT_StageInfo`（Content Browser 的 Settings 中启用 Show Plugin Content）。也可以在 Content Browser → Miscellaneous 创建 **Side Scroller Stage Table**，命名 `DT_StageInfo`。填写任意数量的行，例如：

| RowName | StageName | LevelName |
| --- | --- | --- |
| Stage001 | 第一关 | /Game/SideScroller/Maps/L_Stage01 |
| Stage002 | 第二关 | /Game/SideScroller/Maps/L_Stage02 |

也可以将 `Data/StageInfo.example.csv` 拖入 Content Browser，选择 DataTable 导入类型及 SideScrollerStageRow 行结构。普通 UDataTable 和专用子类都能绑定，只要行结构一致。该 CSV 有 25 条示例，可测试第二页；示例 Level 路径是占位，**没有生成 25 张地图**，必须替换为自己的地图。

LevelName 使用 `/Game/.../L_Stage01`，不要加 `.umap`，也不要使用对象引用形式 `/Game/.../L_Stage01.L_Stage01`。短名称也可传给 OpenLevel，但完整包路径更明确。StageName 为 None 时，界面回退显示 RowName。空表会显示提示并禁用有效选择。

## 选择关卡 Level 的配置

1. 创建选择地图，例如 `L_StageSelect`。
2. 从 SideScrollerStageSelectController 派生 `BP_StageSelectController`。Class Defaults → Stage Select → Stage Table 指定 DT_StageInfo。
3. Stage Select Widget Class 默认已指定原生 Widget，无需更改。
4. 从 SideScrollerStageSelectGameMode 派生 `BP_StageSelectGameMode`，Player Controller Class 设为 BP_StageSelectController。Default Pawn Class 保持 None，不需要在菜单地图生成游戏角色。
5. L_StageSelect 的 World Settings → GameMode Override 设为 BP_StageSelectGameMode。
6. Gameplay 地图应配置自己的 Gameplay GameMode/Controller，不要继续使用菜单 GameMode，否则切图后会重新打开关卡列表。
7. Play 并点击视口取得键盘焦点。使用 ↑/↓ 选择，空格确认。长按上下支持系统按键重复。

## 分页与输入

- 使用 UMG UVerticalBox，创建并复用 20 个 UTextBlock 行控件。
- 当前页显示最多 20 条有效数据，末页不足 20 条的槽位留空。
- 当前选择用黄色文字和 `>` 光标标记。上下键越过第 20/21 条边界时自动翻页。
- 首尾不循环，顶部/底部继续按键保持原选择。
- 底部显示页码、总关卡数和操作提示。
- Controller 直接用 InputComponent.BindKey，不需要额外 Enhanced Input 资源；使用 GameOnly 输入模式，按键由 Controller 处理，避免与 UMG 焦点导航重复响应。
- Space 取得当前选中行，直接调用 UGameplayStatics.OpenLevel。地图不存在或 TravelFailure 时显示错误信息。
- 这次没有实现加载画面、跨地图状态保存、解锁条件或远景流送。

## 打包

LevelName 是 FString，不会自动建立到地图的软/硬资源依赖。请在 Project Settings → Packaging → List of maps to include in a packaged build 中加入选择地图和所有目标 Gameplay 地图，或通过项目现有 Cook 规则包含它们。

这是单机菜单流程；OpenLevel 是本地切图，不作为联网 ServerTravel 流程使用。

## 验证工程

Validation/StageSelectHost 为独立验证宿主，通过 AdditionalPluginDirectories 引用本插件。测试名称为 `SideScroller.StageSelect.PaginationAndDataBinding`，检查数据排序、20 行 UI、跨页选择、选中 LevelName、末页空行及空数据。

示例 CSV 也作为可编辑的数据源保留，实际关卡地图需由你的游戏工程提供。

示例 DT_StageInfo.uasset 已生成，可随插件一起复制，不必重新导入 CSV。仅目标游戏地图仍需在实际工程中创建。若需重新生成缺失的示例表，可启用 Python Editor Script Plugin 和 Editor Scripting Utilities，然后在编辑器执行 `Tools/CreateExampleStageTable.py`；已有同路径表时脚本不覆盖。

本次测试确认：25 条数据稳定排序为两页；VerticalBox 包含 20 个复用行；上下跨页正确；选中 StageName/LevelName 正确；末页多余行为空；首尾边界、空表和无效表安全处理。没有执行到真实 Gameplay 地图的切换，也未进行人工界面排版或打包验证。
