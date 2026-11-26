#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SideScrollerEnemyReferenceAsset.generated.h"

USTRUCT(BlueprintType)
struct SIDESCROLLERRUNTIME_API FSideScrollerEnemyEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Id = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FPrimaryAssetId MeshAssetId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<AActor> EnemyClass;
};

UCLASS(BlueprintType, ClassGroup = (SideScroller))
class SIDESCROLLERRUNTIME_API USideScrollerEnemyReferenceAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	TArray<FSideScrollerEnemyEntry> Enemies;

	bool ValidateCatalog(FString& OutError) const;
	const FSideScrollerEnemyEntry* FindEnemy(int32 Id) const;
};
