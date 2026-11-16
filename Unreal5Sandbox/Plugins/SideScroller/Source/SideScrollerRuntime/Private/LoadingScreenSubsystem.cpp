#include "LoadingScreenSubsystem.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "LoadingWidget.h"

DEFINE_LOG_CATEGORY_STATIC(LogSideScrollerLoading, Log, All);

ULoadingScreenSubsystem::ULoadingScreenSubsystem()
{
    LoadingWidgetClass = ULoadingWidget::StaticClass();
}

bool ULoadingScreenSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
    return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

bool ULoadingScreenSubsystem::StartDebugLoading(APlayerController* PlayerController)
{
    UWorld* World = GetWorld();
    if (!World || World->GetNetMode() == NM_DedicatedServer || !LoadingWidgetClass ||
        !FMath::IsFinite(DebugLoadTime) || DebugLoadTime < 0.0f ||
        !FMath::IsFinite(FadeOutTime) || FadeOutTime < 0.0f)
    {
        UE_LOG(LogSideScrollerLoading, Warning, TEXT("Cannot start loading: invalid world, widget class or duration."));
        return false;
    }
    if (!PlayerController) PlayerController = World->GetFirstPlayerController();
    if (!IsValid(PlayerController) || PlayerController->GetWorld() != World || !PlayerController->IsLocalController())
    {
        UE_LOG(LogSideScrollerLoading, Warning, TEXT("StartDebugLoading needs a local Controller in this World."));
        return false;
    }

    // A repeated start replaces the previous simulation instead of stacking widgets/timers.
    CancelDebugLoading();
    LoadingWidget = CreateWidget<ULoadingWidget>(PlayerController, LoadingWidgetClass);
    if (!LoadingWidget || !LoadingWidget->AddToPlayerScreen(1000))
    {
        CancelDebugLoading();
        UE_LOG(LogSideScrollerLoading, Warning, TEXT("Could not add LoadingWidget to the local player's viewport."));
        return false;
    }

    OwningController = PlayerController;
    ActiveLoadTime = DebugLoadTime;
    ActiveFadeTime = FadeOutTime;
    Progress = 0.0f;
    Phase = EDebugLoadingPhase::Loading;
    LoadStartedAt = World->GetTimeSeconds();
    LoadingWidget->SetProgress(0.0f);
    LoadingWidget->SetRenderOpacity(1.0f);
    World->GetTimerManager().SetTimer(UpdateTimer, this, &ULoadingScreenSubsystem::UpdateLoading, 1.0f / 60.0f, true);
    UpdateLoading();
    return true;
}

void ULoadingScreenSubsystem::UpdateLoading()
{
    UWorld* World = GetWorld();
    if (!World || !IsValid(LoadingWidget) || !OwningController.IsValid())
    {
        CancelDebugLoading();
        return;
    }
    const double Now = World->GetTimeSeconds();
    if (Phase == EDebugLoadingPhase::Loading)
    {
        Progress = ActiveLoadTime <= 0.0f ? 1.0f : FMath::Clamp(static_cast<float>((Now - LoadStartedAt) / ActiveLoadTime), 0.0f, 1.0f);
        LoadingWidget->SetProgress(Progress);
        if (Progress < 1.0f) return;
        Phase = EDebugLoadingPhase::FadingOut;
        FadeStartedAt = Now;
    }
    if (Phase == EDebugLoadingPhase::FadingOut)
    {
        const float FadeAlpha = ActiveFadeTime <= 0.0f ? 1.0f : FMath::Clamp(static_cast<float>((Now - FadeStartedAt) / ActiveFadeTime), 0.0f, 1.0f);
        LoadingWidget->SetRenderOpacity(1.0f - FadeAlpha);
        if (FadeAlpha >= 1.0f) FinishLoading();
    }
}

void ULoadingScreenSubsystem::FinishLoading()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(UpdateTimer);
    if (LoadingWidget) LoadingWidget->RemoveFromParent();
    LoadingWidget = nullptr;
    OwningController.Reset();
    Phase = EDebugLoadingPhase::Idle;
    Progress = 1.0f;
    // Clear state first so subscribers may safely start another simulation.
    OnLoadingFinished.Broadcast();
}

void ULoadingScreenSubsystem::CancelDebugLoading()
{
    if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(UpdateTimer);
    if (LoadingWidget) LoadingWidget->RemoveFromParent();
    LoadingWidget = nullptr;
    OwningController.Reset();
    Phase = EDebugLoadingPhase::Idle;
    Progress = 0.0f;
}

void ULoadingScreenSubsystem::OnWorldEndPlay(UWorld& InWorld)
{
    CancelDebugLoading();
    Super::OnWorldEndPlay(InWorld);
}

void ULoadingScreenSubsystem::Deinitialize()
{
    CancelDebugLoading();
    OnLoadingFinished.Clear();
    Super::Deinitialize();
}
