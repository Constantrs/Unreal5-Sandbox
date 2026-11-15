"""Run inside Unreal Editor after compiling SideScroller; never overwrites an existing asset."""
from pathlib import Path

import unreal


def main():
    asset_path = "/SideScroller/Data/DT_StageInfo"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        unreal.log("Stage table already exists; leaving it unchanged: " + asset_path)
        return

    row_struct = unreal.load_object(None, "/Script/SideScrollerRuntime.SideScrollerStageRow")
    if row_struct is None:
        raise RuntimeError("Compile and enable the SideScroller plugin before creating its table.")

    table = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "DT_StageInfo", "/SideScroller/Data",
        unreal.SideScrollerStageTable, unreal.SideScrollerStageTableFactory())
    if table is None:
        raise RuntimeError("Could not create DT_StageInfo.")

    csv_path = Path(__file__).resolve().parents[1] / "Data" / "StageInfo.example.csv"
    if not unreal.DataTableFunctionLibrary.fill_data_table_from_csv_file(table, str(csv_path), row_struct):
        raise RuntimeError("Could not import the example stage rows.")

    if not unreal.get_editor_subsystem(unreal.EditorAssetSubsystem).save_loaded_asset(table, False):
        raise RuntimeError("Could not save DT_StageInfo.")

    unreal.log("Created " + asset_path + " with 25 example rows. Replace placeholder LevelName values before use.")


main()
