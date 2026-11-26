#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SideScrollerEnemyReferenceAsset.h"
#include "SideScrollerTriggerRefAsset.h"
#include "SideScrollerEnemySpawnRow.h"
#include "SideScrollerStageConfigAsset.h"
#include "SideScrollerAssetLoadSubsystem.generated.h"

struct FStreamableHandle;
class USideScrollerCharaDataAsset;
class USkeletalMesh;

UENUM(BlueprintType)
enum class ESideScrollerAssetLoadState : uint8
{
	Unknown, Queued, LoadingStageConfig, LoadingDefinitions, LoadingMeshes, Ready, Failed, Cancelled
};

USTRUCT(BlueprintType)
struct SIDESCROLLERRUNTIME_API FSideScrollerLoadedEnemy
{
	GENERATED_BODY()
	
	UPROPERTY(Transient, BlueprintReadOnly)
	FSideScrollerEnemyEntry Entry;

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<UClass> Class;

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<USideScrollerCharaDataAsset> Definition;

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<USkeletalMesh> Mesh;
};

USTRUCT(BlueprintType)
struct SIDESCROLLERRUNTIME_API FSideScrollerLoadedTrigger
{
	GENERATED_BODY()
	
	UPROPERTY(Transient, BlueprintReadOnly)
	FSideScrollerTriggerEntry Entry;

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<UClass> Class;
};

USTRUCT(BlueprintType)
struct SIDESCROLLERRUNTIME_API FSideScrollerStageLoadResult
{
	GENERATED_BODY()
	UPROPERTY(Transient, BlueprintReadOnly)
	FName StageId;

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<USideScrollerEnemyReferenceAsset> EnemyAssets;

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<USideScrollerTriggerRefAsset> TriggerAssets;

	UPROPERTY(Transient, BlueprintReadOnly)
	TObjectPtr<UDataTable> EnemySpawnTable;

	UPROPERTY(Transient, BlueprintReadOnly)
	TArray<FSideScrollerEnemySpawnRow> SpawnRows;

	UPROPERTY(Transient, BlueprintReadOnly)
	TMap<int32, FSideScrollerLoadedEnemy> Enemies;

	UPROPERTY(Transient, BlueprintReadOnly)
	TMap<int32, FSideScrollerLoadedTrigger> Triggers;
};

USTRUCT()
struct FSideScrollerAssetLoadBatch
{
	GENERATED_BODY()
	UPROPERTY(Transient)
	TObjectPtr<USideScrollerStageConfigAsset> Configs;

	UPROPERTY(Transient)
	FSideScrollerStageConfig PendingConfig;

	UPROPERTY(Transient)
	FSideScrollerStageLoadResult Result;

	ESideScrollerAssetLoadState State = ESideScrollerAssetLoadState::Queued;
	FString Error;
	TSharedPtr<FStreamableHandle> ConfigHandle;
	TSharedPtr<FStreamableHandle> CatalogsHandle;
	TSharedPtr<FStreamableHandle> DefinitionsHandle;
	TSharedPtr<FStreamableHandle> MeshesHandle;
	TSet<int32> EnemyIds;
	TSet<int32> TriggerIds;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSideScrollerAssetBatchEvent, int32, RequestId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSideScrollerAssetBatchFailedEvent, int32, RequestId, const FString&, Reason);

/** Owns world-local load batches. Never spawns Actors or manipulates loading UI. */
UCLASS(ClassGroup = (SideScroller))
class SIDESCROLLERRUNTIME_API USideScrollerAssetLoadSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
    /** Returns an independent batch ID. Work/events start no earlier than the next world tick. */
    UFUNCTION(BlueprintCallable, Category = "SideScroller|Assets")
    int32 LoadStageAssets(USideScrollerStageConfigAsset* Configs, FName StageId);

	UFUNCTION(BlueprintCallable, Category = "SideScroller|Assets")
	int32 LoadObjectAssets(USideScrollerEnemyReferenceAsset* EnemyCatalog, USideScrollerTriggerRefAsset* TriggerCatalog, UDataTable* SpawnTable);
	
    UFUNCTION(BlueprintPure, Category = "SideScroller|Assets")
    ESideScrollerAssetLoadState GetLoadState(int32 RequestId) const;

    UFUNCTION(BlueprintPure, Category = "SideScroller|Assets")
    float GetLoadProgress(int32 RequestId) const;

    UFUNCTION(BlueprintPure, Category = "SideScroller|Assets")
    FString GetLoadError(int32 RequestId) const;

    UFUNCTION(BlueprintCallable, Category = "SideScroller|Assets")
    bool CopyLoadedResult(int32 RequestId, FSideScrollerStageLoadResult& OutResult) const;

    /** Cancels queued/in-progress work. Retains a Cancelled status until ReleaseBatch. */
    UFUNCTION(BlueprintCallable, Category = "SideScroller|Assets")
    bool CancelBatch(int32 RequestId);

    /** Releases only this batch's handles/cache. Cancels unfinished work silently; never destroys Actors. */
    UFUNCTION(BlueprintCallable, Category = "SideScroller|Assets")
    bool ReleaseBatch(int32 RequestId);

    UPROPERTY(BlueprintAssignable, Category = "SideScroller|Assets")
	FSideScrollerAssetBatchEvent OnBatchReady;

	UPROPERTY(BlueprintAssignable, Category = "SideScroller|Assets")
	FSideScrollerAssetBatchFailedEvent OnBatchFailed;

	UPROPERTY(BlueprintAssignable, Category = "SideScroller|Assets")
	FSideScrollerAssetBatchEvent OnBatchCancelled;

    virtual void OnWorldEndPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

protected:
    virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

private:
    int32 CreateBatch();
	void BeginStageLoad(int32 Id);
	void OnConfigLoaded(int32 Id);
	
	void BeginObjectLoad(int32 Id);
	void OnCatalogsLoaded(int32 Id);
	
	void BeginDefinitionsLoad(int32 Id);
	void OnDefinitionsLoaded(int32 Id);
	
    void OnMeshesLoaded(int32 Id);
    void CompleteBatch(int32 Id);
    void FailBatch(int32 Id, const FString& Reason);
    static void ClearBatchResources(FSideScrollerAssetLoadBatch& Batch);
    void ShutdownBatches();

    UPROPERTY(Transient) TMap<int32, FSideScrollerAssetLoadBatch> Batches;
    int32 LastRequestId = 0;
    bool bShuttingDown = false;
};
