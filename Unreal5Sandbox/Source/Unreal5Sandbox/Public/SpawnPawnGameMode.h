// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "UObject/PrimaryAssetId.h"
#include "SpawnPawnGameMode.generated.h"

class AActorPawn;
class UAssetLoadSubsystem;

/**
 * 
 */
UCLASS()
class UNREAL5SANDBOX_API ASpawnPawnGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void RestartPlayer(AController* NewPlayer) override;
	
	// Select an ActorPawn-derived Blueprint; validated before loading or spawning.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Assets")
	TSubclassOf<APawn> TargetPawn;
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character Assets", meta = (AllowedTypes = "SubMeshDataAsset"))
	FPrimaryAssetId MeshAssetId;

private:
	UFUNCTION()
	void HandlePawnEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason);

	// One successful acquisition per spawned Pawn, including shared resource IDs.
	UPROPERTY(Transient)
	TMap<TObjectPtr<AActorPawn>, FPrimaryAssetId> PawnAssetIds;

	TWeakObjectPtr<UAssetLoadSubsystem> AssetLoadSubsystem;
};