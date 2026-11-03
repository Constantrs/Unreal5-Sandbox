// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SubMeshDataAsset.generated.h"

class UAnimInstance;

USTRUCT(BlueprintType)
struct FSkeletonAssetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skeleton Asset")
	FName AssetName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skeleton Asset")
	TSoftObjectPtr<USkeletalMesh> SkeletonAsset;
};

/**
 * 
 */
UCLASS()
class UNREAL5SANDBOX_API USubMeshDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	/** Index 0 is the leader; all remaining entries follow its bone transforms. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (TitleProperty = "AssetName"))
	TArray<FSkeletonAssetData> SubMeshAssets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TSoftClassPtr<UAnimInstance> AnimClass;
	
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(FPrimaryAssetType(TEXT("SubMeshDataAsset")), GetFName());
	}
};
