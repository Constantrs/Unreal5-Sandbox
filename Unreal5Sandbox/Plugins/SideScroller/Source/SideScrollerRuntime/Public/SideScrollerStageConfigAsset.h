#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SideScrollerStageConfigAsset.generated.h"

class UDataTable;
class USideScrollerEnemyReferenceAsset;
class USideScrollerTriggerRefAsset;

USTRUCT(BlueprintType)
struct SIDESCROLLERRUNTIME_API FSideScrollerStageConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName StageId = NAME_None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<USideScrollerEnemyReferenceAsset> EnemyAssets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<USideScrollerTriggerRefAsset> TriggerAssets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage", meta = (DisplayName = "Object Spawn Table"))
	TSoftObjectPtr<UDataTable> EnemySpawnTable;
	
	/** Menu destination and direct PIE lookup. Reading this reference does not load the map. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UWorld> Level;
};

/**
 * 
 */
UCLASS(ClassGroup = (SideScroller))
class SIDESCROLLERRUNTIME_API USideScrollerStageConfigAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stage", meta = (TitleProperty = "StageId"))
	TArray<FSideScrollerStageConfig> Stages;

	static FString BuildStageOptions(FName Id);
	
	static bool ParseStageOptions(const FString& Options, FName& OutId, FString& OutError);
	
	bool ValidateStageIds(FString& OutError) const;
	
	bool ResolveStage(FName RequestedId, const FString& MapPackageName, FName& OutId,
		FSideScrollerStageConfig& OutConfig, FString& OutError) const;
};
