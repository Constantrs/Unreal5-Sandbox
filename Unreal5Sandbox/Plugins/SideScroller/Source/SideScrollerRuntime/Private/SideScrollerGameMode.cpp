#include "SideScrollerGameMode.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "LoadingScreenSubsystem.h"
#include "SideScrollerGameState.h"
#include "SideScrollerObject.h"
#include "SideScrollerObjectComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogSideScrollerGameMode, Log, All);

ASideScrollerGameMode::ASideScrollerGameMode()
{
    GameStateClass = ASideScrollerGameState::StaticClass();
}

void ASideScrollerGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    USideScrollerStageConfigAsset::ParseStageOptions(Options, StageId, StageOptionError);
    UE_LOG(LogSideScrollerGameMode, Display, TEXT("StageId=%s"), *StageId.ToString());
}

ESideScrollerLoadState ASideScrollerGameMode::GetLoadState() const
{
    if (LoadState != ESideScrollerLoadState::LoadingStageConfig &&
    LoadState != ESideScrollerLoadState::LoadingDefinitions &&
    LoadState != ESideScrollerLoadState::LoadingMeshes)
    {
        return LoadState;
    }
    
    const USideScrollerAssetLoadSubsystem* Loader = GetWorld() ? GetWorld()->GetSubsystem<USideScrollerAssetLoadSubsystem>() : nullptr;
    if (!Loader) return LoadState;
    switch (Loader->GetLoadState(AssetLoadRequestId))
    {
        case ESideScrollerAssetLoadState::Queued:
        case ESideScrollerAssetLoadState::LoadingStageConfig: return ESideScrollerLoadState::LoadingStageConfig;
        case ESideScrollerAssetLoadState::LoadingDefinitions: return ESideScrollerLoadState::LoadingDefinitions;
        case ESideScrollerAssetLoadState::LoadingMeshes: return ESideScrollerLoadState::LoadingMeshes;
        default: return LoadState;
    }
}

bool ASideScrollerGameMode::ReleaseStageAssets()
{
    if (!AssetLoadRequestId || LoadState == ESideScrollerLoadState::Spawning)
    {
        return false;
    }
    const bool bWasLoading = LoadState != ESideScrollerLoadState::Ready &&
        LoadState != ESideScrollerLoadState::Failed && LoadState != ESideScrollerLoadState::Cancelled;
    
    ClearAssetRequest();
    GetWorldTimerManager().ClearTimer(LoadProgressTimer);
    if (bWasLoading)
    {
        LoadState = ESideScrollerLoadState::Cancelled;
        EnemyLoadError = TEXT("Stage assets were released before completion.");
        if (ULoadingScreenSubsystem* Loading = GetWorld()->GetSubsystem<ULoadingScreenSubsystem>())
        {
            Loading->CancelDebugLoading();
        }
        if (!bEndingPlay)
        {
            OnObjectsLoadFailed(EnemyLoadError);
        }
    }
    return true;
}


void ASideScrollerGameMode::OnObjectsReady()
{
    
}

void ASideScrollerGameMode::OnObjectsLoadFailed(const FString& Reason)
{
    
}

void ASideScrollerGameMode::StartStageConfigLoading()
{
    if (LoadState != ESideScrollerLoadState::Uninitialized || bEndingPlay)
    {
        return;
    }
    if (!StageOptionError.IsEmpty())
    {
        FailObjectsLoading(StageOptionError);
        return;
    }
    
    USideScrollerAssetLoadSubsystem* Loader = BindAssetLoader();
    AssetLoadRequestId = Loader ? Loader->LoadStageAssets(StageConfigs, StageId) : 0;
    if (!AssetLoadRequestId)
    {
        FailObjectsLoading(TEXT("Cannot start stage resource request.")); return;
    }
    LoadState = ESideScrollerLoadState::LoadingStageConfig;
    GetWorldTimerManager().SetTimer(LoadProgressTimer, this, &ASideScrollerGameMode::UpdateLoadingProgress, 0.02f, true);
}

void ASideScrollerGameMode::StartObjectLoading(USideScrollerEnemyReferenceAsset* EnemyCatalog,
    USideScrollerTriggerRefAsset* TriggerCatalog, UDataTable* Table)
{
    if (LoadState != ESideScrollerLoadState::Uninitialized || bEndingPlay)
    {
        return;
    }
    USideScrollerAssetLoadSubsystem* Loader = BindAssetLoader();
    AssetLoadRequestId = Loader ? Loader->LoadObjectAssets(EnemyCatalog, TriggerCatalog, Table) : 0;
    if (!AssetLoadRequestId)
    {
        FailObjectsLoading(TEXT("Cannot start enemy resource request.")); return;
    }
    LoadState = ESideScrollerLoadState::LoadingDefinitions;
    GetWorldTimerManager().SetTimer(LoadProgressTimer, this, &ASideScrollerGameMode::UpdateLoadingProgress, 0.02f, true);
}

void ASideScrollerGameMode::BeginPlay()
{
    Super::BeginPlay();
    if (GetNetMode() == NM_DedicatedServer)
    {
        return;
    }
    TryStartLoading();
}

void ASideScrollerGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    bEndingPlay = true;
    GetWorldTimerManager().ClearTimer(LoadProgressTimer);
    if (USideScrollerAssetLoadSubsystem* Loader = GetWorld()->GetSubsystem<USideScrollerAssetLoadSubsystem>())
    {
        Loader->OnBatchReady.RemoveDynamic(this, &ASideScrollerGameMode::HandleAssetsReady);
        Loader->OnBatchFailed.RemoveDynamic(this, &ASideScrollerGameMode::HandleAssetsFailed);
        Loader->OnBatchCancelled.RemoveDynamic(this, &ASideScrollerGameMode::HandleAssetsCancelled);
    }
    ClearAssetRequest();
    SpawnData = FSideScrollerStageLoadResult();
    SpawnedObjects.Reset();
    Super::EndPlay(EndPlayReason);
}

USideScrollerAssetLoadSubsystem* ASideScrollerGameMode::BindAssetLoader()
{
    USideScrollerAssetLoadSubsystem* Loader = GetWorld()->GetSubsystem<USideScrollerAssetLoadSubsystem>();
    if (Loader)
    {
        Loader->OnBatchReady.AddUniqueDynamic(this, &ASideScrollerGameMode::HandleAssetsReady);
        Loader->OnBatchFailed.AddUniqueDynamic(this, &ASideScrollerGameMode::HandleAssetsFailed);
        Loader->OnBatchCancelled.AddUniqueDynamic(this, &ASideScrollerGameMode::HandleAssetsCancelled);
    }
    return Loader;
}

void ASideScrollerGameMode::TryStartLoading()
{
    UWorld* World = GetWorld();
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!IsValid(PC) || !PC->IsLocalController())
        {
            continue;
        }
        const ULocalPlayer* Player = PC->GetLocalPlayer();
        if (!Player || !Player->ViewportClient)
        {
            continue;
        }
        
        ULoadingScreenSubsystem* Loading = GetWorld()->GetSubsystem<ULoadingScreenSubsystem>();
        if (!Loading || !(bUseDebugLoading ? Loading->StartDebugLoading(PC) : Loading->StartLoading(PC)))
        {
            FailObjectsLoading(TEXT("Cannot create the loading screen."));
            return;
        }
        if (!bUseDebugLoading)
        {
            StartStageConfigLoading();
        }
        return;
    }
}

void ASideScrollerGameMode::SpawnObjects()
{
    LoadState = ESideScrollerLoadState::Spawning;
    for (const FSideScrollerEnemySpawnRow& Row : SpawnData.SpawnRows)
    {
        const bool bTrigger = Row.ObjectInfo.Category == ESideScrollerCategory::TGR;
        UClass* SpawnClass = nullptr;
        USkeletalMesh* ExpectedMesh = nullptr;
        if (bTrigger)
        {
            if (const FSideScrollerLoadedTrigger* Loaded = SpawnData.Triggers.Find(Row.ObjectInfo.Id)) SpawnClass = Loaded->Class;
        }
        else if (Row.ObjectInfo.Category == ESideScrollerCategory::EM)
        {
            if (const FSideScrollerLoadedEnemy* Loaded = SpawnData.Enemies.Find(Row.ObjectInfo.Id))
            {
                SpawnClass = Loaded->Class;
                ExpectedMesh = Loaded->Mesh;
            }
        }
        if (!SpawnClass || (!bTrigger && !ExpectedMesh))
        {
            FailObjectsLoading(TEXT("Spawn row has no loaded object configuration."));
            return;
        }
        AActor* Actor = GetWorld()->SpawnActorDeferred<AActor>(SpawnClass, Row.Transform, nullptr, nullptr,
            bTrigger ? ESpawnActorCollisionHandlingMethod::AlwaysSpawn : ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding);
        if (!Actor)
        {
            FailObjectsLoading(FString::Printf(TEXT("Cannot spawn Category=%d Id=%d (check spawn collision)."), static_cast<int32>(Row.ObjectInfo.Category), Row.ObjectInfo.Id));
            return;
        }
        SpawnedObjects.Add(Actor);
        USideScrollerObjectComponent* Object = Actor->FindComponentByClass<USideScrollerObjectComponent>();
        if (!Object)
        {
            FailObjectsLoading(TEXT("Spawned object requires SideScrollerObjectComponent.")); return;
        }
        Object->SetCategory(Row.ObjectInfo.Category);
        if (!bTrigger)
        {
            Object->SetRootMeshAsset(ExpectedMesh);
        }
        Actor->FinishSpawning(Row.Transform);
        if (bEndingPlay)
        {
            return;
        }
        Object = Actor->FindComponentByClass<USideScrollerObjectComponent>();
        if (!IsValid(Actor) || Actor->IsActorBeingDestroyed() || !Object ||
            ISideScrollerObject::Execute_GetCategory(Actor) != Row.ObjectInfo.Category)
        {
            FailObjectsLoading(TEXT("Object initialization failed: verify category and ObjectComponent."));
            return;
        }
        if (!bTrigger)
        {
            USkeletalMeshComponent* Mesh = ISideScrollerObject::Execute_GetRootSkeletalMeshComponent(Actor);
            if (!Mesh || Mesh->GetOwner() != Actor || Mesh->GetSkeletalMeshAsset() != ExpectedMesh)
            {
                FailObjectsLoading(TEXT("Enemy initialization failed: verify root mesh."));
                return;
            }
        }
        if (!Object->RegisterWithGameState()) { FailObjectsLoading(TEXT("Object registration failed: verify GameState.")); return; }
    }
    LoadState = ESideScrollerLoadState::Ready;
    GetWorldTimerManager().ClearTimer(LoadProgressTimer);
    UE_LOG(LogSideScrollerGameMode, Display, TEXT("Stage object initialization complete: %d instances, %d Enemy types, %d Trigger types."),
        SpawnedObjects.Num(), SpawnData.Enemies.Num(), SpawnData.Triggers.Num());
    SpawnData = FSideScrollerStageLoadResult(); // Loader retains the batch; GameMode drops its temporary copy.
    OnObjectsReady();
    if (!bEndingPlay)
    {
        if (ULoadingScreenSubsystem* Loading = GetWorld()->GetSubsystem<ULoadingScreenSubsystem>()) Loading->CompleteLoading();   
    }
}

void ASideScrollerGameMode::UpdateLoadingProgress()
{
    USideScrollerAssetLoadSubsystem* Loader = GetWorld()->GetSubsystem<USideScrollerAssetLoadSubsystem>();
    if (!Loader || Loader->GetLoadState(AssetLoadRequestId) == ESideScrollerAssetLoadState::Unknown)
    {
        FailObjectsLoading(TEXT("Asset request was released before completion."));
        return;
    }
    if (ULoadingScreenSubsystem* Loading = GetWorld()->GetSubsystem<ULoadingScreenSubsystem>())
    {
        Loading->SetLoadingProgress(FMath::Max(Loading->GetProgress(), 0.9f * Loader->GetLoadProgress(AssetLoadRequestId)));   
    }
}

void ASideScrollerGameMode::FailObjectsLoading(const FString& Reason)
{
    LoadState = ESideScrollerLoadState::Failed;
    EnemyLoadError = Reason;
    GetWorldTimerManager().ClearTimer(LoadProgressTimer);
    for (const TWeakObjectPtr<AActor>& Actor : SpawnedObjects) if (Actor.IsValid()) Actor->Destroy();
    SpawnedObjects.Reset();
    SpawnData = FSideScrollerStageLoadResult();
    ClearAssetRequest();
    if (ULoadingScreenSubsystem* Loading = GetWorld()->GetSubsystem<ULoadingScreenSubsystem>()) Loading->CancelDebugLoading();
    UE_LOG(LogSideScrollerGameMode, Error, TEXT("Enemy loading failed: %s"), *EnemyLoadError);
    if (!bEndingPlay)
    {
        OnObjectsLoadFailed(EnemyLoadError);
    }
}

void ASideScrollerGameMode::ClearAssetRequest()
{
    const int32 Id = AssetLoadRequestId;
    AssetLoadRequestId = 0;
    if (USideScrollerAssetLoadSubsystem* Loader = GetWorld()->GetSubsystem<USideScrollerAssetLoadSubsystem>()) Loader->ReleaseBatch(Id);
    EnemyRefAssets.Reset();
    TriggerRefAssets.Reset();
    ObjectSpawnTable.Reset();
}

void ASideScrollerGameMode::HandleAssetsReady(int32 RequestId)
{
    if (bEndingPlay || RequestId != AssetLoadRequestId)
    {
        return;
    }
    
    USideScrollerAssetLoadSubsystem* Loader = GetWorld()->GetSubsystem<USideScrollerAssetLoadSubsystem>();
    if (!Loader || !Loader->CopyLoadedResult(RequestId, SpawnData))
    {
        FailObjectsLoading(TEXT("Loaded result was released before spawning."));
        return;
    }
    
    StageId = SpawnData.StageId.IsNone() ? StageId : SpawnData.StageId;
    EnemyRefAssets = SpawnData.EnemyAssets;
    TriggerRefAssets = SpawnData.TriggerAssets;
    ObjectSpawnTable = SpawnData.EnemySpawnTable;
    SpawnObjects();
}

void ASideScrollerGameMode::HandleAssetsFailed(int32 RequestId, const FString& Reason)
{
     if (!bEndingPlay && RequestId == AssetLoadRequestId)
     {
         FailObjectsLoading(Reason);
     }
}

void ASideScrollerGameMode::HandleAssetsCancelled(int32 RequestId)
{
    if (bEndingPlay || RequestId != AssetLoadRequestId)
    {
        return;
    }
    LoadState = ESideScrollerLoadState::Cancelled;
    EnemyLoadError = TEXT("Asset load request was cancelled.");
    GetWorldTimerManager().ClearTimer(LoadProgressTimer);
    ClearAssetRequest();
    if (ULoadingScreenSubsystem* Loading = GetWorld()->GetSubsystem<ULoadingScreenSubsystem>())
    {
        Loading->CancelDebugLoading();
    }
    if (!bEndingPlay) OnObjectsLoadFailed(EnemyLoadError);
}
