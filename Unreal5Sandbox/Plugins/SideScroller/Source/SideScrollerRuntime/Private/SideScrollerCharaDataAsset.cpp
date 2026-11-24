#include "SideScrollerCharaDataAsset.h"

const FPrimaryAssetType USideScrollerCharaDataAsset::PrimaryAssetType = FPrimaryAssetType(TEXT("SideScrollerCharaDataAsset"));
	
FPrimaryAssetId USideScrollerCharaDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(PrimaryAssetType, GetFName());
}
