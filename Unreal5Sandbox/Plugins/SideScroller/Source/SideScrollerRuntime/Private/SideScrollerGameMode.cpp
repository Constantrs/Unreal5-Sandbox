#include "SideScrollerGameMode.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/LocalPlayer.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StreamableManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "LoadingScreenSubsystem.h"
#include "SideScrollerGameState.h"
#include "SideScrollerCharaDataAsset.h"
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
    StageId = FName(*UGameplayStatics::ParseOption(Options, TEXT("StageId")));
    // StageId is diagnostic for now; each map's GameMode Blueprint supplies the table/catalog.
    UE_LOG(LogSideScrollerGameMode, Display, TEXT("StageId=%s"), *StageId.ToString());
}

void ASideScrollerGameMode::OnEnemiesReady()
{
}

void ASideScrollerGameMode::OnEnemyLoadFailed(const FString& Reason)
{
    
}

void ASideScrollerGameMode::StartEnemyLoading()
{
    if (EnemyLoadState != ESideScrollerEnemyLoadState::Idle || bEndingPlay)
    {
        return;
    }
    
    if (!EnemySpawnTable || EnemySpawnTable->GetRowStruct() != FSideScrollerEnemySpawnRow::StaticStruct())
    {
        FailEnemyLoading(TEXT("Assign EnemySpawnTable with SideScrollerEnemySpawnRow rows."));
        return;
    }
    // Snapshot data before async work; subsequent table edits cannot change this batch.
    TArray<FName> Names = EnemySpawnTable->GetRowNames();
    Names.Sort(FNameLexicalLess());
    TArray<FSideScrollerObjectInfo> Infos;
    for (FName Name : Names)
    {
        const FSideScrollerEnemySpawnRow* Row = EnemySpawnTable->FindRow<FSideScrollerEnemySpawnRow>(Name, TEXT("Enemy loading"));
        if (!Row || Row->ObjectInfo.Category != ESideScrollerCategory::EM) continue;
        if (!Row->Transform.IsValid())
        {
            FailEnemyLoading(FString::Printf(TEXT("Invalid transform in spawn row %s."), *Name.ToString()));
            return;
        }
        SpawnRows.Add(*Row);
        Infos.Add(Row->ObjectInfo);
    }
    LoadAssetsFromScrollerObjectInfo(Infos);
}

void ASideScrollerGameMode::LoadAssetsFromScrollerObjectInfo(const TArray<FSideScrollerObjectInfo>& Infos)
{
    EnemyLoadState = ESideScrollerEnemyLoadState::LoadingDefinitions;
    ++LoadRequestId;
    if (Infos.IsEmpty())
    {
        SpawnEnemies();
        return;
    }
    FString Error;
    if (!EnemyRefAssets || !EnemyRefAssets->ValidateCatalog(Error))
    {
        FailEnemyLoading(EnemyRefAssets ? Error : TEXT("Assign EnemyAssets catalog."));
        return;
    }

    UAssetManager& Manager = UAssetManager::Get();
    TArray<FSoftObjectPath> Paths;
    for (const FSideScrollerObjectInfo& Info : Infos)
    {
        if (Info.Category != ESideScrollerCategory::EM || LoadedEnemies.Contains(Info.Id)) continue;
        const FSideScrollerEnemyEntry* Entry = EnemyRefAssets->FindEnemy(Info.Id);
        if (!Entry)
        {
            FailEnemyLoading(FString::Printf(TEXT("Enemy Id=%d was not found."), Info.Id));
            return;
        }
        const FSoftObjectPath DefinitionPath = Manager.GetPrimaryAssetPath(Entry->MeshAssetId);
        if (!DefinitionPath.IsValid())
        {
            FailEnemyLoading(FString::Printf(TEXT("Unknown PrimaryAssetId %s; check Asset Manager scanning."), *Entry->MeshAssetId.ToString()));
            return;
        }
        FSideScrollerLoadedEnemy& Loaded = LoadedEnemies.Add(Info.Id);
        Loaded.Entry = *Entry;
        Paths.AddUnique(DefinitionPath);
        Paths.AddUnique(Entry->EnemyClass.ToSoftObjectPath());
    }
    // Resolve primary IDs with AssetManager, then retain our own streamable handles.
    // Avoid global UnloadPrimaryAssets, which could affect another consumer of the same ID.
    const uint32 RequestId = LoadRequestId;
    DefinitionsHandle = Manager.GetStreamableManager().RequestAsyncLoad(Paths,
        FStreamableDelegate::CreateWeakLambda(this, [this, RequestId] { OnDefinitionsLoaded(RequestId); }));
    if (!DefinitionsHandle)
    {
        FailEnemyLoading(TEXT("Could not request enemy definitions/classes."));
        return;
    }
    GetWorldTimerManager().SetTimer(LoadProgressTimer, this, &ASideScrollerGameMode::UpdateLoadingProgress, 0.05f, true);
}

void ASideScrollerGameMode::BeginPlay()
{
    Super::BeginPlay();
    if (GetNetMode() == NM_DedicatedServer) return;
    //GetWorldTimerManager().SetTimer(WaitForPlayerTimer, this, &ASideScrollerGameMode::TryStartLoading, 0.1f, true);
    TryStartLoading();
}

void ASideScrollerGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
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
            StartEnemyLoading();
        }
        return;
    }
}

void ASideScrollerGameMode::OnDefinitionsLoaded(uint32 RequestId)
{
    if (bEndingPlay || RequestId != LoadRequestId || EnemyLoadState != ESideScrollerEnemyLoadState::LoadingDefinitions) return;
    TArray<FSoftObjectPath> MeshPaths;
    for (auto& Pair : LoadedEnemies)
    {
        FSideScrollerLoadedEnemy& Loaded = Pair.Value;
        Loaded.Class = Loaded.Entry.EnemyClass.Get();
        Loaded.Definition = Cast<USideScrollerCharaDataAsset>(UAssetManager::Get().GetPrimaryAssetPath(Loaded.Entry.MeshAssetId).ResolveObject());
        if (!Loaded.Class || !Loaded.Class->IsChildOf(AActor::StaticClass()) ||
            Loaded.Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists) ||
            !Loaded.Class->ImplementsInterface(USideScrollerObject::StaticClass()) ||
            !Loaded.Definition || Loaded.Definition->SkeletalAssetRef.SkeletonAsset.IsNull())
        {
            FailEnemyLoading(FString::Printf(TEXT("Enemy Id=%d: failed class/definition load, invalid interface or missing mesh."), Pair.Key));
            return;
        }
        MeshPaths.AddUnique(Loaded.Definition->SkeletalAssetRef.SkeletonAsset.ToSoftObjectPath());
    }
    EnemyLoadState = ESideScrollerEnemyLoadState::LoadingMeshes;
    UpdateLoadingProgress();
    MeshesHandle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(MeshPaths,
        FStreamableDelegate::CreateWeakLambda(this, [this, RequestId] { OnMeshesLoaded(RequestId); }));
    if (!MeshesHandle) FailEnemyLoading(TEXT("Could not request enemy meshes."));
}

void ASideScrollerGameMode::OnMeshesLoaded(uint32 RequestId)
{
    if (bEndingPlay || RequestId != LoadRequestId || EnemyLoadState != ESideScrollerEnemyLoadState::LoadingMeshes) return;
    for (auto& Pair : LoadedEnemies)
    {
        Pair.Value.Mesh = Pair.Value.Definition->SkeletalAssetRef.SkeletonAsset.Get();
        if (!Pair.Value.Mesh)
        {
            FailEnemyLoading(FString::Printf(TEXT("SkeletalMesh load failed for Enemy Id=%d."), Pair.Key));
            return;
        }
    }
    SpawnEnemies();
}

void ASideScrollerGameMode::SpawnEnemies()
{
     EnemyLoadState = ESideScrollerEnemyLoadState::Spawning;
    UpdateLoadingProgress();
    for (const FSideScrollerEnemySpawnRow& Row : SpawnRows)
    {
        const FSideScrollerLoadedEnemy* Loaded = LoadedEnemies.Find(Row.ObjectInfo.Id);
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
        if (!Object)
        {
            FailEnemyLoading(TEXT("Enemy requires SideScrollerObjectComponent."));
            return;
        }
        Object->SetCategory(ESideScrollerCategory::EM);
        Object->SetRootMeshAsset(Loaded->Mesh);
        Enemy->FinishSpawning(Row.Transform);
        if (bEndingPlay) return;
        // Construction scripts may replace components or override defaults: verify the final object.
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
    EnemyLoadState = ESideScrollerEnemyLoadState::Ready;
    GetWorldTimerManager().ClearTimer(LoadProgressTimer);
    // Strong UPROPERTY references now retain classes/definitions/meshes for this stage.
    ReleaseLoadHandles();
    UE_LOG(LogSideScrollerGameMode, Display, TEXT("Enemy initialization complete: %d instances, %d types."), SpawnedEnemies.Num(), LoadedEnemies.Num());
    OnEnemiesReady();
    if (!bEndingPlay)
    {
        if (ULoadingScreenSubsystem* Loading = GetWorld()->GetSubsystem<ULoadingScreenSubsystem>()) Loading->CompleteLoading();
    }
}

void ASideScrollerGameMode::UpdateLoadingProgress()
{
    float Progress = 0.0f;
    if (EnemyLoadState == ESideScrollerEnemyLoadState::LoadingDefinitions)
    {
        Progress = 0.4f * (DefinitionsHandle ? DefinitionsHandle->GetProgress() : 0.0f);   
    }
    else if (EnemyLoadState == ESideScrollerEnemyLoadState::LoadingMeshes)
    {
        Progress = 0.4f + 0.5f * (MeshesHandle ? MeshesHandle->GetProgress() : 0.0f);
    }
    else if (EnemyLoadState == ESideScrollerEnemyLoadState::Spawning)
    {
        Progress = 0.9f;   
    }
    if (ULoadingScreenSubsystem* Loading = GetWorld()->GetSubsystem<ULoadingScreenSubsystem>())
    {
        Loading->SetLoadingProgress(FMath::Clamp(Progress, Loading->GetProgress(), 0.99f));   
    }
}

void ASideScrollerGameMode::FailEnemyLoading(const FString& Reason)
{
    ++LoadRequestId;
    EnemyLoadState = ESideScrollerEnemyLoadState::Failed;
    EnemyLoadError = Reason;
    GetWorldTimerManager().ClearTimer(LoadProgressTimer);
    ReleaseLoadHandles();
    for (const TWeakObjectPtr<AActor>& Enemy : SpawnedEnemies)
        if (Enemy.IsValid()) Enemy->Destroy();
    SpawnedEnemies.Reset();
    LoadedEnemies.Reset();
    if (ULoadingScreenSubsystem* Loading = GetWorld()->GetSubsystem<ULoadingScreenSubsystem>()) Loading->CancelDebugLoading();
    UE_LOG(LogSideScrollerGameMode, Error, TEXT("Enemy loading failed: %s"), *Reason);
    OnEnemyLoadFailed(Reason);
}

void ASideScrollerGameMode::ReleaseLoadHandles()
{
    for (TSharedPtr<FStreamableHandle>* Handle : { &DefinitionsHandle, &MeshesHandle })
    {
        if (Handle->IsValid())
        {
            if (!(*Handle)->HasLoadCompleted()) (*Handle)->CancelHandle();
            else (*Handle)->ReleaseHandle();
            Handle->Reset();
        }
    }
}
