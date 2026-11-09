#include "SpawnPawnGameMode.h"
#include "ActorPawn.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerController.h"
#include "AssetLoadSubsystem.h"
#include "ActorDataAsset.h"

DEFINE_LOG_CATEGORY_STATIC(LogActorGameMode, Log, All);

void ASpawnPawnGameMode::RestartPlayer(AController* NewPlayer)
{
	if (!HasAuthority() || !IsValid(NewPlayer) || NewPlayer->IsActorBeingDestroyed()) return;
    // RestartPlayer can be called repeatedly; never create a second controlled Pawn.
    if (NewPlayer->GetPawn()) return;

    if (!TargetPawn || !TargetPawn->IsChildOf(AActorPawn::StaticClass()))
    {
        UE_LOG(LogActorGameMode, Error, TEXT("TargetPawn must be set to an ActorPawn-derived class."));
        return;
    }
    
    AActor* start = FindPlayerStart(NewPlayer);
    if (!IsValid(start))
    {
        UE_LOG(LogActorGameMode, Error, TEXT("No PlayerStart found for %s."), *NewPlayer->GetName());
        return;
    }

    UGameInstance* instance = GetGameInstance();
    UAssetLoadSubsystem* subsystem = instance ? instance->GetSubsystem<UAssetLoadSubsystem>() : nullptr;
    if (!subsystem) return;

    // Capture the ID used by this request so its eventual release matches acquisition.
    const FPrimaryAssetId requestedId = MeshAssetId;
    UActorDataAsset* data = subsystem->AcquireActorAssetSync(requestedId);
    if (!data) return;

    FActorSpawnParameters params;
    params.Owner = NewPlayer;
    params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    const FRotator spawnRotation(0.0f, start->GetActorRotation().Yaw, 0.0f);
    AActorPawn* pawn = GetWorld()->SpawnActor<AActorPawn>(
        TargetPawn.Get(), start->GetActorLocation(), spawnRotation, params);

    if (!IsValid(pawn) || !pawn->InitializeActorMesh(data))
    {
        if (IsValid(pawn)) pawn->Destroy();
        subsystem->ReleaseActorAsset(requestedId);
        UE_LOG(LogActorGameMode, Error, TEXT("Failed to spawn or initialize ActorPawn."));
        return;
    }

    AssetLoadSubsystem = subsystem;
    PawnAssetIds.Add(pawn, requestedId);
    pawn->OnEndPlay.AddDynamic(this, &ASpawnPawnGameMode::HandlePawnEndPlay);

    NewPlayer->Possess(pawn);
    if (NewPlayer->GetPawn() != pawn)
    {
        // EndPlay callback releases this Pawn's acquisition.
        pawn->Destroy();
        UE_LOG(LogActorGameMode, Error, TEXT("Controller failed to possess the spawned Pawn."));
        return;
    }

    NewPlayer->SetControlRotation(FRotator(-25.0f, spawnRotation.Yaw, 0.0f));
    if (APlayerController* Player = Cast<APlayerController>(NewPlayer))
    {
        Player->ClientSetRotation(NewPlayer->GetControlRotation(), true);
    }
    SetPlayerDefaults(pawn);
    K2_OnRestartPlayer(NewPlayer);
}

void ASpawnPawnGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // World teardown may end GameMode before its Pawns. Release each acquisition once.
    for (const auto& Pair : PawnAssetIds)
    {
        if (IsValid(Pair.Key.Get()))
            Pair.Key->OnEndPlay.RemoveDynamic(this, &ASpawnPawnGameMode::HandlePawnEndPlay);
        if (UAssetLoadSubsystem* subsystem = AssetLoadSubsystem.Get())
            subsystem->ReleaseActorAsset(Pair.Value);
    }
    PawnAssetIds.Empty();
    AssetLoadSubsystem.Reset();
    Super::EndPlay(EndPlayReason);
}

void ASpawnPawnGameMode::HandlePawnEndPlay(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
    AActorPawn* Pawn = Cast<AActorPawn>(Actor);
    FPrimaryAssetId AssetId;
    if (Pawn && PawnAssetIds.RemoveAndCopyValue(Pawn, AssetId))
    {
        if (UAssetLoadSubsystem* subsystem = AssetLoadSubsystem.Get())
            subsystem->ReleaseActorAsset(AssetId);
    }
}