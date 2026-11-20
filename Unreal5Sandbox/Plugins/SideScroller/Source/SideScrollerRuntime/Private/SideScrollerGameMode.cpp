#include "SideScrollerGameMode.h"

#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "LoadingScreenSubsystem.h"
#include "SideScrollerCustomOptions.h"
#include "SideScrollerGameState.h"
#include "SideScrollerObjectTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogSideScrollerGameMode, Log, All);

ASideScrollerGameMode::ASideScrollerGameMode()
{
    GameStateClass = ASideScrollerGameState::StaticClass();
}

void ASideScrollerGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);

    FString ParseError;
    const SideScrollerCustomOptions::EParseResult Result =
        SideScrollerCustomOptions::ParseCustomOptions(Options, ObjectInfos, ParseError);
    bHasObjectInfoOptions = Result == SideScrollerCustomOptions::EParseResult::Success;
    if (Result == SideScrollerCustomOptions::EParseResult::Invalid)
    {
        UE_LOG(LogSideScrollerGameMode, Warning, TEXT("%s"), *ParseError);
        return;
    }
    if (Result == SideScrollerCustomOptions::EParseResult::NotProvided)
    {
        UE_LOG(LogSideScrollerGameMode, Display, TEXT("SCObjects was not provided; object list is empty."));
        return;
    }
    UE_LOG(LogSideScrollerGameMode, Display, TEXT("Received %d object configurations."), ObjectInfos.Num());
    for (const FSideScrollerObjectInfo& Info : ObjectInfos)
    {
        UE_LOG(LogSideScrollerGameMode, Display, TEXT("Category=%d, Id=%d"), static_cast<int32>(Info.Category), Info.Id);
    }
}

void ASideScrollerGameMode::BeginPlay()
{
    Super::BeginPlay();

    // GameMode runs only on the authority; this loading UI is for standalone play.
    if (GetNetMode() == NM_DedicatedServer) return;

    TryStartLoading();
    
    // Defer UI creation until after BeginPlay and wait for the local player if needed.
    // GetWorldTimerManager().SetTimer(
    //     WaitForPlayerTimer, this, &ASideScrollerGameMode::TryStartLoading, 0.1f, true);
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
    Super::EndPlay(EndPlayReason);
}
