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

ESideScrollerLoadState ASideScrollerGameMode::GetEnemyLoadState() const
{
    if (EnemyLoadState != ESideScrollerLoadState::LoadingStageConfig &&
    EnemyLoadState != ESideScrollerLoadState::LoadingDefinitions &&
    EnemyLoadState != ESideScrollerLoadState::LoadingMeshes)
    {
        return EnemyLoadState;
    }
    
    const USideScrollerAssetLoadSubsystem* Loader = GetWorld() ? GetWorld()->GetSubsystem<USideScrollerAssetLoadSubsystem>() : nullptr;
    if (!Loader) return EnemyLoadState;
    switch (Loader->GetLoadState(AssetLoadRequestId))
    {
        case ESideScrollerAssetLoadState::Queued:
        case ESideScrollerAssetLoadState::LoadingStageConfig: return ESideScrollerLoadState::LoadingStageConfig;
        case ESideScrollerAssetLoadState::LoadingDefinitions: return ESideScrollerLoadState::LoadingDefinitions;
        case ESideScrollerAssetLoadState::LoadingMeshes: return ESideScrollerLoadState::LoadingMeshes;
        default: return EnemyLoadState;
    }
}

bool ASideScrollerGameMode::ReleaseStageAssets()
{
    if (!AssetLoadRequestId || EnemyLoadState == ESideScrollerLoadState::Spawning)
    {
        return false;
    }
    const bool bWasLoading = EnemyLoadState != ESideScrollerLoadState::Ready &&
        EnemyLoadState != ESideScrollerLoadState::Failed && EnemyLoadState != ESideScrollerLoadState::Cancelled;
    
    ClearAssetRequest();
    GetWorldTimerManager().ClearTimer(LoadProgressTimer);
    if (bWasLoading)
    {
        EnemyLoadState = ESideScrollerLoadState::Cancelled;
        EnemyLoadError = TEXT("Stage assets were released before completion.");
        if (ULoadingScreenSubsystem* Loading = GetWorld()->GetSubsystem<ULoadingScreenSubsystem>())
        {
            Loading->CancelDebugLoading();
        }
        OnEnemyLoadFailed(EnemyLoadError);
    }
    return true;

}

void ASideScrollerGameMode::OnEnemiesReady()
{
    
}

void ASideScrollerGameMode::OnEnemyLoadFailed(const FString& Reason)
{
    
}

void ASideScrollerGameMode::StartStageConfigLoading()
{
    if (EnemyLoadState != ESideScrollerLoadState::Uninitialized || bEndingPlay)
    {
        return;
    }
    if (!StageOptionError.IsEmpty())
    {
        FailEnemyLoading(StageOptionError);
        return;
    }
    
    USideScrollerAssetLoadSubsystem* Loader = BindAssetLoader();
    AssetLoadRequestId = Loader ? Loader->LoadStageAssets(StageConfigs, StageId) : 0;
    if (!AssetLoadRequestId)
    {
        FailEnemyLoading(TEXT("Cannot start stage resource request.")); return;
    }
    EnemyLoadState = ESideScrollerLoadState::LoadingStageConfig;
    GetWorldTimerManager().SetTimer(LoadProgressTimer, this, &ASideScrollerGameMode::UpdateLoadingProgress, 0.02f, true);
}

void ASideScrollerGameMode::StartEnemyLoading(USideScrollerEnemyReferenceAsset* Catalog, UDataTable* Table)
{
    if (EnemyLoadState != ESideScrollerLoadState::Uninitialized || bEndingPlay)
    {
        return;
    }
    USideScrollerAssetLoadSubsystem* Loader = BindAssetLoader();
    AssetLoadRequestId = Loader ? Loader->LoadEnemyAssets(Catalog, Table) : 0;
    if (!AssetLoadRequestId)
    {
        FailEnemyLoading(TEXT("Cannot start enemy resource request.")); return;
    }
    EnemyLoadState = ESideScrollerLoadState::LoadingDefinitions;
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
    SpawnedEnemies.Reset();
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
            FailEnemyLoading(TEXT("Cannot create the loading screen."));
            return;
        }
        if (!bUseDebugLoading)
        {
            StartStageConfigLoading();
        }
        return;
    }
}

void ASideScrollerGameMode::SpawnEnemies()
{
    EnemyLoadState = ESideScrollerLoadState::Spawning;
    for (const FSideScrollerEnemySpawnRow& Row : SpawnData.SpawnRows)
    {
        const FSideScrollerLoadedEnemy* Loaded = SpawnData.Enemies.Find(Row.ObjectInfo.Id);
        if (!Loaded || !Loaded->Class || !Loaded->Mesh)
        {
            FailEnemyLoading(TEXT("Spawn row has no loaded enemy configuration."));
            return;
        }
        AActor* Enemy = GetWorld()->SpawnActorDeferred<AActor>(Loaded->Class, Row.Transform, nullptr, nullptr,
            ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding);
        if (!Enemy)
        {
            FailEnemyLoading(FString::Printf(TEXT("Cannot spawn Enemy Id=%d (check spawn collision)."), Row.ObjectInfo.Id));
            return;
        }
        SpawnedEnemies.Add(Enemy);
        USideScrollerObjectComponent* Object = Enemy->FindComponentByClass<USideScrollerObjectComponent>();
        if (!Object) { FailEnemyLoading(TEXT("Enemy requires SideScrollerObjectComponent.")); return; }
        Object->SetCategory(ESideScrollerCategory::EM);
        Object->SetRootMeshAsset(Loaded->Mesh);
        Enemy->FinishSpawning(Row.Transform);
        if (bEndingPlay) return;
        Object = Enemy->FindComponentByClass<USideScrollerObjectComponent>();
        USkeletalMeshComponent* Mesh = IsValid(Enemy) ? ISideScrollerObject::Execute_GetRootSkeletalMeshComponent(Enemy) : nullptr;
        if (!IsValid(Enemy) || Enemy->IsActorBeingDestroyed() || !Object || !Mesh || Mesh->GetOwner() != Enemy ||
            Mesh->GetSkeletalMeshAsset() != Loaded->Mesh ||
            ISideScrollerObject::Execute_GetCategory(Enemy) != ESideScrollerCategory::EM || !Object->RegisterWithGameState())
        {
            FailEnemyLoading(TEXT("Enemy initialization failed: verify root mesh, category and GameState."));
            return;
        }
    }
    EnemyLoadState = ESideScrollerLoadState::Ready;
    GetWorldTimerManager().ClearTimer(LoadProgressTimer);
    UE_LOG(LogSideScrollerGameMode, Display, TEXT("Enemy initialization complete: %d instances, %d types."), SpawnedEnemies.Num(), SpawnData.Enemies.Num());
    SpawnData = FSideScrollerStageLoadResult(); // Loader retains the batch; GameMode drops its temporary copy.
    OnEnemiesReady();
    if (!bEndingPlay)
    {
        if (ULoadingScreenSubsystem* Loading = GetWorld()->GetSubsystem<ULoadingScreenSubsystem>())
        {
            Loading->CompleteLoading();
        }
    }
}

void ASideScrollerGameMode::UpdateLoadingProgress()
{
    USideScrollerAssetLoadSubsystem* Loader = GetWorld()->GetSubsystem<USideScrollerAssetLoadSubsystem>();
    if (!Loader || Loader->GetLoadState(AssetLoadRequestId) == ESideScrollerAssetLoadState::Unknown)
    {
        FailEnemyLoading(TEXT("Asset request was released before completion."));
        return;
    }
    if (ULoadingScreenSubsystem* Loading = GetWorld()->GetSubsystem<ULoadingScreenSubsystem>())
    {
        Loading->SetLoadingProgress(FMath::Max(Loading->GetProgress(), 0.9f * Loader->GetLoadProgress(AssetLoadRequestId)));   
    }
}

void ASideScrollerGameMode::FailEnemyLoading(const FString& Reason)
{
    EnemyLoadState = ESideScrollerLoadState::Failed;
    EnemyLoadError = Reason;
    GetWorldTimerManager().ClearTimer(LoadProgressTimer);
    for (const TWeakObjectPtr<AActor>& Enemy : SpawnedEnemies) if (Enemy.IsValid()) Enemy->Destroy();
    SpawnedEnemies.Reset();
    SpawnData = FSideScrollerStageLoadResult();
    ClearAssetRequest();
    if (ULoadingScreenSubsystem* Loading = GetWorld()->GetSubsystem<ULoadingScreenSubsystem>()) Loading->CancelDebugLoading();
    UE_LOG(LogSideScrollerGameMode, Error, TEXT("Enemy loading failed: %s"), *EnemyLoadError);
    OnEnemyLoadFailed(EnemyLoadError);
}

void ASideScrollerGameMode::ClearAssetRequest()
{
    const int32 Id = AssetLoadRequestId;
    AssetLoadRequestId = 0;
    if (USideScrollerAssetLoadSubsystem* Loader = GetWorld()->GetSubsystem<USideScrollerAssetLoadSubsystem>()) Loader->ReleaseBatch(Id);
    EnemyRefAssets.Reset();
    EnemySpawnTable.Reset();
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
        FailEnemyLoading(TEXT("Loaded result was released before spawning."));
        return;
    }
    StageId = SpawnData.StageId.IsNone() ? StageId : SpawnData.StageId;
    EnemyRefAssets = SpawnData.EnemyAssets;
    EnemySpawnTable = SpawnData.EnemySpawnTable;
    SpawnEnemies();

}

void ASideScrollerGameMode::HandleAssetsFailed(int32 RequestId, const FString& Reason)
{
     if (!bEndingPlay && RequestId == AssetLoadRequestId)
     {
         FailEnemyLoading(Reason);
     }
}

void ASideScrollerGameMode::HandleAssetsCancelled(int32 RequestId)
{
    if (bEndingPlay || RequestId != AssetLoadRequestId)
    {
        return;
    }
    EnemyLoadState = ESideScrollerLoadState::Cancelled;
    EnemyLoadError = TEXT("Asset load request was cancelled.");
    GetWorldTimerManager().ClearTimer(LoadProgressTimer);
    ClearAssetRequest();
    if (ULoadingScreenSubsystem* Loading = GetWorld()->GetSubsystem<ULoadingScreenSubsystem>())
    {
        Loading->CancelDebugLoading();
    }
    OnEnemyLoadFailed(EnemyLoadError);

}
