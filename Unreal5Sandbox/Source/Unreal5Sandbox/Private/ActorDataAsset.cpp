// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorDataAsset.h"

FPrimaryAssetId UActorDataAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType(TEXT("ActorDataAsset")), GetFName());
}

USkeletalMesh* UActorDataAsset::FindSkeletonAsset(FName AssetName) const
{
	if (AssetName.IsNone()) return nullptr;
	const FActorSkeletonAssetEntry* Entry = SkeletonAssets.FindByPredicate(
		[AssetName](const FActorSkeletonAssetEntry& Item) { return Item.AssetName == AssetName; });
	return Entry ? Entry->SkeletonAsset.Get() : nullptr;
}

UAnimMontage* UActorDataAsset::FindAnimationMontage(FName AssetName) const
{
	if (AssetName.IsNone()) return nullptr;
	const FActorMontageAssetEntry* Entry = AnimationMontages.FindByPredicate(
		[AssetName](const FActorMontageAssetEntry& Item) { return Item.AssetName == AssetName; });
	return Entry ? Entry->AnimationMontage.Get() : nullptr;
}