#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ActorDataAsset.generated.h"

class USkeletalMesh;
class UAnimMontage;
class UAnimInstance;

USTRUCT(BlueprintType)
struct FActorSkeletonAssetEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skeleton Asset")
	FName AssetName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skeleton Asset", meta = (AssetBundles = "Skeleton"))
	TSoftObjectPtr<USkeletalMesh> SkeletonAsset;
};

USTRUCT(BlueprintType)
struct FActorMontageAssetEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Montage")
	FName AssetName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Montage", meta = (AssetBundles = "Montage"))
	TSoftObjectPtr<UAnimMontage> AnimationMontage;
};

UCLASS(BlueprintType)
class UNREAL5SANDBOX_API UActorDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// Index 0 may be used as the Leader Mesh, as in SkeletonAssetData.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skeleton Assets", meta = (TitleProperty = "AssetName"))
	TArray<FActorSkeletonAssetEntry> SkeletonAssets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation Montages", meta = (TitleProperty = "AssetName"))
	TArray<FActorMontageAssetEntry> AnimationMontages;

	// Optional Animation Blueprint for the existing modular-mesh initialization flow.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation", meta = (AssetBundles = "Skeleton"))
	TSoftClassPtr<UAnimInstance> AnimClass;
	
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/** Returns an already-loaded mesh. Does not start loading; missing/unloaded returns null. */
	UFUNCTION(BlueprintPure, Category = "Actor Assets")
	USkeletalMesh* FindSkeletonAsset(FName AssetName) const;

	/** Returns an already-loaded Montage. Does not start loading; missing/unloaded returns null. */
	UFUNCTION(BlueprintPure, Category = "Actor Assets")
	UAnimMontage* FindAnimationMontage(FName AssetName) const;
};
