// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SubMeshDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FSkeletonAssetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skeleton Asset")
	FName AssetName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skeleton Asset")
	TObjectPtr<USkeletalMesh> SkeletonAsset = nullptr;
};


/**
 * 
 */
UCLASS()
class UNREAL5SANDBOX_API USubMeshDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	/** Index 0 is the leader; all remaining entries follow its bone transforms. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (TitleProperty = "AssetName"))
	TArray<FSkeletonAssetData> SubMeshAssets;
};
