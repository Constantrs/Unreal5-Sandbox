// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SideScrollerCharaDataAsset.generated.h"

class USkeletalMesh;

USTRUCT(BlueprintType)
struct FCharaSkeletonAssetRef
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName AssetName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AssetBundles = "Skeleton"))
	TSoftObjectPtr<USkeletalMesh> SkeletonAsset;
};


/**
 * 
 */
UCLASS(ClassGroup = (SideScroller))
class SIDESCROLLERRUNTIME_API USideScrollerCharaDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	static const FPrimaryAssetType PrimaryAssetType;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeshAsset", meta = (TitleProperty = "AssetName"))
	FCharaSkeletonAssetRef SkeletalAssetRef;
	
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

};
