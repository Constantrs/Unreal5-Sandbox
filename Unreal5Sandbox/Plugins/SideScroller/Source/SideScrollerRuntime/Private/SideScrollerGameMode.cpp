#include "SideScrollerGameMode.h"

#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "LoadingScreenSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogSideScrollerGameMode, Log, All);

void ASideScrollerGameMode::BeginPlay()
{
    Super::BeginPlay();

    // GameMode runs only on the authority; this loading UI is for standalone play.
    if (GetNetMode() == NM_DedicatedServer) return;

    // Defer UI creation until after BeginPlay and wait for the local player if needed.
    GetWorldTimerManager().SetTimer(
        WaitForPlayerTimer, this, &ASideScrollerGameMode::TryStartLoading, 0.1f, true);
}

void ASideScrollerGameMode::TryStartLoading()
{
    UWorld* World = GetWorld();
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PlayerController = It->Get();
        if (!IsValid(PlayerController) || !PlayerController->IsLocalController()) continue;

        const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
        if (!LocalPlayer || !LocalPlayer->ViewportClient) continue;

        // Attempt once after the viewport is ready; configuration errors should not retry forever.
        GetWorldTimerManager().ClearTimer(WaitForPlayerTimer);
        ULoadingScreenSubsystem* Loading = World->GetSubsystem<ULoadingScreenSubsystem>();
        if (!Loading || !Loading->StartDebugLoading(PlayerController))
        {
            UE_LOG(LogSideScrollerGameMode, Error, TEXT("Failed to start the Gameplay loading screen."));
        }
        return;
    }
}

void ASideScrollerGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(WaitForPlayerTimer);
    Super::EndPlay(EndPlayReason);
}
