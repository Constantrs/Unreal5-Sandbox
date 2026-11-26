#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TimerManager.h"
#include "SideScrollerAssetLoadSubsystem.h"
#include "SideScrollerGameMode.generated.h"

struct FStreamableHandle;
class USideScrollerCharaDataAsset;
class USkeletalMesh;

UENUM(BlueprintType)
enum class ESideScrollerLoadState : uint8
{
    Uninitialized,
    LoadingStageConfig,
    LoadingDefinitions,
    LoadingMeshes,
    Spawning,
    Ready,
    Failed,
    Cancelled
};

/** Single-player stage bootstrap: async resources -> deferred enemy spawning -> loading fade. */
UCLASS(Blueprintable, ClassGroup = (SideScroller), meta = (PrioritizeCategories = "SideScroller"))
class SIDESCROLLERRUNTIME_API ASideScrollerGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    ASideScrollerGameMode();

    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

    UFUNCTION(BlueprintPure, Category = "SideScroller|Enemies")
    ESideScrollerLoadState GetEnemyLoadState() const;

    UFUNCTION(BlueprintPure, Category = "SideScroller|Enemies")
    FString GetEnemyLoadError() const { return EnemyLoadError; }
    
    UFUNCTION(BlueprintPure, Category = "Stage|Assets")
    int32 GetAssetLoadRequestId() const { return AssetLoadRequestId; }

    /** Does not destroy enemies. Unfinished loads are cancelled; cannot release mid-spawn. */
    UFUNCTION(BlueprintCallable, Category = "Stage|Assets")
    bool ReleaseStageAssets();

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SideScroller|Enemies")
    TObjectPtr<USideScrollerStageConfigAsset> StageConfigs;
    
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SideScroller|Enemies")
    TWeakObjectPtr<USideScrollerEnemyReferenceAsset> EnemyRefAssets;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SideScroller|Enemies")
    TWeakObjectPtr<UDataTable> EnemySpawnTable;
    
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SideScroller|Stage")
    TArray<FSideScrollerObjectInfo> ObjectInfos;

    /** Optional legacy UI simulation; skips all enemy loading/spawning. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SideScroller|Debug")
    bool bUseDebugLoading = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SideScroller|Stage")
    FName StageId;
    
    virtual void OnEnemiesReady();
    
    virtual void OnEnemyLoadFailed(const FString& Reason);

    void StartStageConfigLoading();
    void StartEnemyLoading(USideScrollerEnemyReferenceAsset* Catalog, UDataTable* Table);
    
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
private:
    USideScrollerAssetLoadSubsystem* BindAssetLoader();
    
    void TryStartLoading();
    void SpawnEnemies();
    void UpdateLoadingProgress();
    void FailEnemyLoading(const FString& Reason);
    void ClearAssetRequest();
    
    UFUNCTION()
    void HandleAssetsReady(int32 RequestId);

    UFUNCTION()
    void HandleAssetsFailed(int32 RequestId, const FString& Reason);

    UFUNCTION()
    void HandleAssetsCancelled(int32 RequestId);
    
    FTimerHandle LoadProgressTimer;
    int32 AssetLoadRequestId = 0;
    bool bEndingPlay = false;
    ESideScrollerLoadState EnemyLoadState = ESideScrollerLoadState::Uninitialized;
    FString EnemyLoadError;
    FString StageOptionError;

    // Only retained while synchronous spawn/Blueprint initialization is executing.
    UPROPERTY(Transient)
    FSideScrollerStageLoadResult SpawnData;

    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<AActor>> SpawnedEnemies;
};

