#include "SideScrollerCharaDataAsset.h"

FPrimaryAssetType USideScrollerCharaDataAsset::PrimaryAssetType = FPrimaryAssetType(TEXT("SideScrollerCharaDataAsset"));
	
FPrimaryAssetId USideScrollerCharaDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(PrimaryAssetType, GetFName());
}
