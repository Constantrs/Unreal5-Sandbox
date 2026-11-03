// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/PrimaryAssetId.h"
#include "AssetLoadSubsystem.generated.h"

class USubMeshDataAsset;

USTRUCT()
struct FLoadedCharacterAsset
{
	GENERATED_BODY()

	UPROPERTY(Transient)
	TObjectPtr<USubMeshDataAsset> SubMeshAsset;

	// Soft references do not retain loaded objects. This cache does.
	UPROPERTY(Transient)
	TArray<TObjectPtr<UObject>> Resources;

	int32 Users = 0;
};

/**
 * 
 */
UCLASS()
class UNREAL5SANDBOX_API UAssetLoadSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	// Game-thread only. Each successful acquisition must be released once.
	UFUNCTION(BlueprintCallable, Category = "Character Assets")
	USubMeshDataAsset* AcquireCharacterAssetSync(FPrimaryAssetId AssetId);

	UFUNCTION(BlueprintCallable, Category = "Character Assets")
	void ReleaseCharacterAsset(FPrimaryAssetId AssetId);

	virtual void Deinitialize() override;
	
private:
	UPROPERTY(Transient)
	TMap<FPrimaryAssetId, FLoadedCharacterAsset> LoadedAssets;
};
