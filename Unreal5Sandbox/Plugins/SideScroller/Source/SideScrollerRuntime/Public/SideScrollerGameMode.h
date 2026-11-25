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
enum class ESideScrollerEnemyLoadState : uint8
{
    Idle, LoadingStageConfig, LoadingDefinitions, LoadingMeshes, Spawning, Ready, Failed, Cancelled
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
    ESideScrollerEnemyLoadState GetEnemyLoadState() const { return EnemyLoadState; }

    UFUNCTION(BlueprintPure, Category = "SideScroller|Enemies")
    FString GetEnemyLoadError() const { return EnemyLoadError; }
    
protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "SideScroller|Enemies")
    TObjectPtr<USideScrollerStageConfigAsset> StageConfigs;
    
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SideScroller|Enemies")
    TObjectPtr<USideScrollerEnemyReferenceAsset> EnemyRefAssets;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "SideScroller|Enemies")
    TObjectPtr<UDataTable> EnemySpawnTable;
    
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
    
    void StartEnemyLoading();
    
    /** Starts one batch. Non-enemy entries are ignored. */
    void LoadAssetsFromScrollerObjectInfo(const TArray<FSideScrollerObjectInfo>& Infos);
    
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
private:
    void TryStartLoading();
    void OnStageConfigLoaded(uint32 RequestId);
    void OnDefinitionsLoaded(uint32 RequestId);
    
    void OnMeshesLoaded(uint32 RequestId);
    void SpawnEnemies();
    void UpdateLoadingProgress();
    void FailEnemyLoading(const FString& Reason);
    void ReleaseLoadHandles();
    
    FTimerHandle LoadProgressTimer;
    TSharedPtr<FStreamableHandle> DefinitionsHandle;
    TSharedPtr<FStreamableHandle> StageConfigHandle;
    TSharedPtr<FStreamableHandle> MeshesHandle;
    uint32 LoadRequestId = 0;
    bool bEndingPlay = false;

    UPROPERTY(Transient)
    TMap<int32, FSideScrollerLoadedEnemy> LoadedEnemies;

    UPROPERTY(Transient)
    TArray<FSideScrollerEnemySpawnRow> SpawnRows;

    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<AActor>> SpawnedEnemies;

    ESideScrollerEnemyLoadState EnemyLoadState = ESideScrollerEnemyLoadState::Idle;
    FString EnemyLoadError;
    FString StageOptionError;

    UPROPERTY(Transient)
    FSideScrollerStageConfig PendingStageConfig;
};

