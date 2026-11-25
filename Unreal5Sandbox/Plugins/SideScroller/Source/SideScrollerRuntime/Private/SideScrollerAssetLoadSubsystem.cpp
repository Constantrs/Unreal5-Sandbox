#include "SideScrollerAssetLoadSubsystem.h"

#include "Engine/AssetManager.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "SideScrollerCharaDataAsset.h"
#include "SideScrollerObject.h"
#include "TimerManager.h"

bool USideScrollerAssetLoadSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

int32 USideScrollerAssetLoadSubsystem::CreateBatch()
{
	if (bShuttingDown || !GetWorld() || LastRequestId == MAX_int32) return 0;
	const int32 Id = ++LastRequestId;
	Batches.Add(Id);
	return Id;
}

int32 USideScrollerAssetLoadSubsystem::LoadStageAssets(USideScrollerStageConfigAsset* Configs, FName StageId)
{
	const int32 Id = CreateBatch();
	if (!Id) return 0;
	Batches[Id].Configs = Configs;
	Batches[Id].Result.StageId = StageId;
	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this, Id] { BeginStageLoad(Id); }));
	return Id;
}

int32 USideScrollerAssetLoadSubsystem::LoadEnemyAssets(USideScrollerEnemyReferenceAsset* Catalog, UDataTable* SpawnTable)
{
    const int32 Id = CreateBatch();
    if (!Id) return 0;
    Batches[Id].Result.EnemyAssets = Catalog;
    Batches[Id].Result.EnemySpawnTable = SpawnTable;
    GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this, Id] { BeginEnemyLoad(Id); }));
    return Id;
}

void USideScrollerAssetLoadSubsystem::BeginStageLoad(int32 Id)
{
    FSideScrollerAssetLoadBatch* Batch = Batches.Find(Id);
    if (bShuttingDown || !Batch || Batch->State != ESideScrollerAssetLoadState::Queued) return;
    FString Error;
    FName ResolvedId;
    if (!Batch->Configs || !Batch->Configs->ResolveStage(Batch->Result.StageId, GetWorld()->GetOutermost()->GetName(),
        ResolvedId, Batch->PendingConfig, Error))
    {
        FailBatch(Id, Batch->Configs ? Error : TEXT("Assign the shared StageConfigs asset in the GameMode Blueprint."));
        return;
    }
    Batch->Result.StageId = ResolvedId;
    Batch->State = ESideScrollerAssetLoadState::LoadingStageConfig;
    TArray<FSoftObjectPath> Paths;
    Paths.AddUnique(Batch->PendingConfig.EnemyAssets.ToSoftObjectPath());
    Paths.AddUnique(Batch->PendingConfig.EnemySpawnTable.ToSoftObjectPath());
    Batch->ConfigHandle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(Paths,
        FStreamableDelegate::CreateWeakLambda(this, [this, Id] { OnConfigLoaded(Id); }));
    if (!Batch->ConfigHandle) FailBatch(Id, TEXT("Could not request stage catalog/spawn table."));
}

void USideScrollerAssetLoadSubsystem::OnConfigLoaded(int32 Id)
{
    FSideScrollerAssetLoadBatch* Batch = Batches.Find(Id);
    if (bShuttingDown || !Batch || Batch->State != ESideScrollerAssetLoadState::LoadingStageConfig) return;
    Batch->Result.EnemyAssets = Batch->PendingConfig.EnemyAssets.Get();
    Batch->Result.EnemySpawnTable = Batch->PendingConfig.EnemySpawnTable.Get();
    if (!Batch->Result.EnemyAssets || !Batch->Result.EnemySpawnTable)
    {
        FailBatch(Id, TEXT("Stage catalog/spawn table failed to load."));
        return;
    }
    BeginEnemyLoad(Id);
}

void USideScrollerAssetLoadSubsystem::BeginEnemyLoad(int32 Id)
{
    FSideScrollerAssetLoadBatch* Batch = Batches.Find(Id);
    if (bShuttingDown || !Batch || (Batch->State != ESideScrollerAssetLoadState::Queued &&
        Batch->State != ESideScrollerAssetLoadState::LoadingStageConfig)) return;
    UDataTable* Table = Batch->Result.EnemySpawnTable;
    if (!Table || Table->GetRowStruct() != FSideScrollerEnemySpawnRow::StaticStruct())
    {
        FailBatch(Id, TEXT("Assign EnemySpawnTable with SideScrollerEnemySpawnRow rows."));
        return;
    }
    TArray<FName> Names = Table->GetRowNames();
    Names.Sort(FNameLexicalLess());
    TSet<int32> EnemyIds;
    for (FName Name : Names)
    {
        const FSideScrollerEnemySpawnRow* Row = Table->FindRow<FSideScrollerEnemySpawnRow>(Name, TEXT("Enemy loading"));
        if (!Row || Row->ObjectInfo.Category != ESideScrollerCategory::EM) continue;
        if (!Row->Transform.IsValid())
        {
            FailBatch(Id, FString::Printf(TEXT("Invalid transform in spawn row %s."), *Name.ToString()));
            return;
        }
        Batch->Result.SpawnRows.Add(*Row);
        EnemyIds.Add(Row->ObjectInfo.Id);
    }
    if (EnemyIds.IsEmpty()) { CompleteBatch(Id); return; }
    FString Error;
    USideScrollerEnemyReferenceAsset* Catalog = Batch->Result.EnemyAssets;
    if (!Catalog || !Catalog->ValidateCatalog(Error))
    {
        FailBatch(Id, Catalog ? Error : TEXT("Assign EnemyAssets catalog."));
        return;
    }
    Batch->State = ESideScrollerAssetLoadState::LoadingDefinitions;
    TArray<FSoftObjectPath> Paths;
    UAssetManager& Manager = UAssetManager::Get();
    for (int32 EnemyId : EnemyIds)
    {
        const FSideScrollerEnemyEntry* Entry = Catalog->FindEnemy(EnemyId);
        if (!Entry) { FailBatch(Id, FString::Printf(TEXT("Enemy Id=%d was not found."), EnemyId)); return; }
        const FSoftObjectPath Path = Manager.GetPrimaryAssetPath(Entry->MeshAssetId);
        if (!Path.IsValid())
        {
            FailBatch(Id, FString::Printf(TEXT("Unknown PrimaryAssetId %s; check Asset Manager scanning."), *Entry->MeshAssetId.ToString()));
            return;
        }
        Batch->Result.Enemies.Add(EnemyId).Entry = *Entry;
        Paths.AddUnique(Path);
        Paths.AddUnique(Entry->EnemyClass.ToSoftObjectPath());
    }
    Batch->DefinitionsHandle = Manager.GetStreamableManager().RequestAsyncLoad(Paths,
        FStreamableDelegate::CreateWeakLambda(this, [this, Id] { OnDefinitionsLoaded(Id); }));
    if (!Batch->DefinitionsHandle) FailBatch(Id, TEXT("Could not request enemy definitions/classes."));
}

void USideScrollerAssetLoadSubsystem::OnDefinitionsLoaded(int32 Id)
{
    FSideScrollerAssetLoadBatch* Batch = Batches.Find(Id);
    if (bShuttingDown || !Batch || Batch->State != ESideScrollerAssetLoadState::LoadingDefinitions) return;
    TArray<FSoftObjectPath> Paths;
    for (auto& Pair : Batch->Result.Enemies)
    {
        FSideScrollerLoadedEnemy& Loaded = Pair.Value;
        Loaded.Class = Loaded.Entry.EnemyClass.Get();
        Loaded.Definition = Cast<USideScrollerCharaDataAsset>(UAssetManager::Get().GetPrimaryAssetPath(Loaded.Entry.MeshAssetId).ResolveObject());
        if (!Loaded.Class || !Loaded.Class->IsChildOf(AActor::StaticClass()) ||
            Loaded.Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists) ||
            !Loaded.Class->ImplementsInterface(USideScrollerObject::StaticClass()) ||
            !Loaded.Definition || Loaded.Definition->SkeletalAssetRef.SkeletonAsset.IsNull())
        {
            FailBatch(Id, FString::Printf(TEXT("Enemy Id=%d: invalid class, interface, definition or mesh."), Pair.Key));
            return;
        }
        Paths.AddUnique(Loaded.Definition->SkeletalAssetRef.SkeletonAsset.ToSoftObjectPath());
    }
    Batch->State = ESideScrollerAssetLoadState::LoadingMeshes;
    Batch->MeshesHandle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(Paths,
        FStreamableDelegate::CreateWeakLambda(this, [this, Id] { OnMeshesLoaded(Id); }));
    if (!Batch->MeshesHandle) FailBatch(Id, TEXT("Could not request enemy meshes."));
}

void USideScrollerAssetLoadSubsystem::OnMeshesLoaded(int32 Id)
{
    FSideScrollerAssetLoadBatch* Batch = Batches.Find(Id);
    if (bShuttingDown || !Batch || Batch->State != ESideScrollerAssetLoadState::LoadingMeshes) return;
    for (auto& Pair : Batch->Result.Enemies)
    {
        Pair.Value.Mesh = Pair.Value.Definition->SkeletalAssetRef.SkeletonAsset.Get();
        if (!Pair.Value.Mesh) { FailBatch(Id, FString::Printf(TEXT("Mesh load failed for Enemy Id=%d."), Pair.Key)); return; }
    }
    CompleteBatch(Id);
}

void USideScrollerAssetLoadSubsystem::CompleteBatch(int32 Id)
{
    FSideScrollerAssetLoadBatch* Batch = Batches.Find(Id);
    if (!Batch || bShuttingDown) return;
    Batch->State = ESideScrollerAssetLoadState::Ready;
    // All handles stay active until the owner explicitly releases this batch.
    OnBatchReady.Broadcast(Id);
}

void USideScrollerAssetLoadSubsystem::FailBatch(int32 Id, const FString& Reason)
{
    FSideScrollerAssetLoadBatch* Batch = Batches.Find(Id);
    if (!Batch || bShuttingDown) return;
    const FString Error = Reason; // Caller may pass data owned by this batch.
    Batch->State = ESideScrollerAssetLoadState::Failed;
    Batch->Error = Error;
    ClearBatchResources(*Batch);
    OnBatchFailed.Broadcast(Id, Error);
}

ESideScrollerAssetLoadState USideScrollerAssetLoadSubsystem::GetLoadState(int32 Id) const
{
    const FSideScrollerAssetLoadBatch* Batch = Batches.Find(Id);
    return Batch ? Batch->State : ESideScrollerAssetLoadState::Unknown;
}

float USideScrollerAssetLoadSubsystem::GetLoadProgress(int32 Id) const
{
    const FSideScrollerAssetLoadBatch* Batch = Batches.Find(Id);
    if (!Batch) return 0.0f;
    switch (Batch->State)
    {
    case ESideScrollerAssetLoadState::LoadingStageConfig: return 0.15f * (Batch->ConfigHandle ? Batch->ConfigHandle->GetProgress() : 0.0f);
    case ESideScrollerAssetLoadState::LoadingDefinitions: return 0.15f + 0.35f * (Batch->DefinitionsHandle ? Batch->DefinitionsHandle->GetProgress() : 0.0f);
    case ESideScrollerAssetLoadState::LoadingMeshes: return 0.5f + 0.5f * (Batch->MeshesHandle ? Batch->MeshesHandle->GetProgress() : 0.0f);
    case ESideScrollerAssetLoadState::Ready: return 1.0f;
    default: return 0.0f;
    }
}

FString USideScrollerAssetLoadSubsystem::GetLoadError(int32 Id) const
{
    const FSideScrollerAssetLoadBatch* Batch = Batches.Find(Id);
    return Batch ? Batch->Error : TEXT("Unknown or released load request.");
}

bool USideScrollerAssetLoadSubsystem::CopyLoadedResult(int32 Id, FSideScrollerStageLoadResult& OutResult) const
{
    OutResult = FSideScrollerStageLoadResult();
    const FSideScrollerAssetLoadBatch* Batch = Batches.Find(Id);
    if (!Batch || Batch->State != ESideScrollerAssetLoadState::Ready) return false;
    OutResult = Batch->Result;
    return true;
}

void USideScrollerAssetLoadSubsystem::ClearBatchResources(FSideScrollerAssetLoadBatch& Batch)
{
    for (TSharedPtr<FStreamableHandle>* Handle : { &Batch.ConfigHandle, &Batch.DefinitionsHandle, &Batch.MeshesHandle })
    {
        if (Handle->IsValid())
        {
            if (!(*Handle)->HasLoadCompleted()) (*Handle)->CancelHandle();
            else (*Handle)->ReleaseHandle();
            Handle->Reset();
        }
    }
    Batch.Result = FSideScrollerStageLoadResult();
    Batch.PendingConfig = FSideScrollerStageConfig();
    Batch.Configs = nullptr;
}

bool USideScrollerAssetLoadSubsystem::CancelBatch(int32 Id)
{
    FSideScrollerAssetLoadBatch* Batch = Batches.Find(Id);
    if (!Batch || bShuttingDown || Batch->State == ESideScrollerAssetLoadState::Ready ||
        Batch->State == ESideScrollerAssetLoadState::Failed || Batch->State == ESideScrollerAssetLoadState::Cancelled) return false;
    Batch->State = ESideScrollerAssetLoadState::Cancelled;
    ClearBatchResources(*Batch);
    OnBatchCancelled.Broadcast(Id);
    return true;
}

bool USideScrollerAssetLoadSubsystem::ReleaseBatch(int32 Id)
{
    FSideScrollerAssetLoadBatch* Batch = Batches.Find(Id);
    if (!Batch) return false;
    // Invalidate state before cancelling callbacks; IDs are never reused in this world.
    Batch->State = ESideScrollerAssetLoadState::Cancelled;
    ClearBatchResources(*Batch);
    Batches.Remove(Id);
    return true;
}

void USideScrollerAssetLoadSubsystem::ShutdownBatches()
{
    bShuttingDown = true;
    for (auto& Pair : Batches) ClearBatchResources(Pair.Value);
    Batches.Reset();
}

void USideScrollerAssetLoadSubsystem::OnWorldEndPlay(UWorld& InWorld)
{
    ShutdownBatches();
    Super::OnWorldEndPlay(InWorld);
}

void USideScrollerAssetLoadSubsystem::Deinitialize()
{
    ShutdownBatches();
    OnBatchReady.Clear();
    OnBatchFailed.Clear();
    OnBatchCancelled.Clear();
    Super::Deinitialize();
}