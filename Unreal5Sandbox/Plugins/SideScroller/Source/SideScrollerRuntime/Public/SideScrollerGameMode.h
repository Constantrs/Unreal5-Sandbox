#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TimerManager.h"
#include "SideScrollerObjectTypes.h"
#include "SideScrollerEnemyReferenceAsset.h"
#include "SideScrollerEnemySpawnRow.h"
#include "SideScrollerGameMode.generated.h"

struct FStreamableHandle;
class USideScrollerCharaDataAsset;
class USkeletalMesh;

UENUM(BlueprintType)
enum class ESideScrollerEnemyLoadState : uint8
{
    Idle, LoadingDefinitions, LoadingMeshes, Spawning, Ready, Failed
};

USTRUCT()
struct FSideScrollerLoadedEnemy
{
    GENERATED_BODY()
    UPROPERTY(Transient) FSideScrollerEnemyEntry Entry;
    UPROPERTY(Transient) TObjectPtr<UClass> Class;
    UPROPERTY(Transient) TObjectPtr<USideScrollerCharaDataAsset> Definition;
    UPROPERTY(Transient) TObjectPtr<USkeletalMesh> Mesh;
};

/** Single-player stage bootstrap: async resources -> deferred enemy spawning -> loading fade. */
UCLASS(Blueprintable,  ClassGroup = (SideScroller), meta = (PrioritizeCategories = "Stage"))
class SIDESCROLLERRUNTIME_API ASideScrollerGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    ASideScrollerGameMode();

    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

    UFUNCTION(BlueprintPure, Category = "Stage|Enemies")
    ESideScrollerEnemyLoadState GetEnemyLoadState() const { return EnemyLoadState; }

    UFUNCTION(BlueprintPure, Category = "Stage|Enemies")
    FString GetEnemyLoadError() const { return EnemyLoadError; }
    
protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stage|Enemies")
    TObjectPtr<USideScrollerEnemyReferenceAsset> EnemyRefAssets;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stage|Enemies", meta = (RequiredAssetDataTags = "RowStructure=/Script/SideScrollerRuntime.SideScrollerEnemySpawnRow"))
    TObjectPtr<UDataTable> EnemySpawnTable;
    
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stage")
    TArray<FSideScrollerObjectInfo> ObjectInfos;

    /** Optional legacy UI simulation; skips all enemy loading/spawning. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stage|Debug")
    bool bUseDebugLoading = false;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stage")
    FName StageId;
    
    virtual void OnEnemiesReady();
    
    virtual void OnEnemyLoadFailed(const FString& Reason);

    void StartEnemyLoading();

    /** Starts one batch. Non-enemy entries are ignored. */
    void LoadAssetsFromScrollerObjectInfo(const TArray<FSideScrollerObjectInfo>& Infos);
    
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
private:
    void TryStartLoading();
    void OnDefinitionsLoaded(uint32 RequestId);
    void OnMeshesLoaded(uint32 RequestId);
    void SpawnEnemies();
    void UpdateLoadingProgress();
    void FailEnemyLoading(const FString& Reason);
    void ReleaseLoadHandles();
    
    FTimerHandle LoadProgressTimer;
    TSharedPtr<FStreamableHandle> DefinitionsHandle;
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
};

