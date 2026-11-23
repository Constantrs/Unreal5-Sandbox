#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TimerManager.h"
#include "LoadingScreenSubsystem.generated.h"

class APlayerController;
class ULoadingWidget;

UENUM(BlueprintType)
enum class EDebugLoadingPhase : uint8
{
    Idle,
    Loading,
    FadingOut
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDebugLoadingFinishedEvent);

/** World-scoped debug loading flow. Uses a world timer rather than a tickable subsystem. */
UCLASS(Config = Game)
class SIDESCROLLERRUNTIME_API ULoadingScreenSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    ULoadingScreenSubsystem();

    /** Starts or restarts the simulation. Null selects the first local Controller in this World. */
    UFUNCTION(BlueprintCallable, Category = "Loading")
    bool StartDebugLoading(APlayerController* PlayerController = nullptr);

    /** External work controls progress and completion; never auto-finishes at 100%. */
    UFUNCTION(BlueprintCallable, Category = "Loading")
    bool StartLoading(APlayerController* PlayerController = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Loading")
    void SetLoadingProgress(float InProgress);

    UFUNCTION(BlueprintCallable, Category = "Loading")
    void CompleteLoading();
    
    /** Removes UI and clears timers without reporting successful completion. */
    UFUNCTION(BlueprintCallable, Category = "Loading")
    void CancelDebugLoading();

    UFUNCTION(BlueprintPure, Category = "Loading")
    bool IsLoading() const { return Phase != EDebugLoadingPhase::Idle; }

    UFUNCTION(BlueprintPure, Category = "Loading")
    float GetProgress() const { return Progress; }

    UFUNCTION(BlueprintPure, Category = "Loading")
    EDebugLoadingPhase GetPhase() const { return Phase; }

    UFUNCTION(BlueprintPure, Category = "Loading")
    ULoadingWidget* GetLoadingWidget() const { return LoadingWidget.Get(); }

    /** Simulation duration in game-time seconds. Zero completes immediately and starts the fade. */
    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Loading|Debug", meta = (ClampMin = "0.0", Units = "s"))
    float DebugLoadTime = 2.0f;

    UPROPERTY(Config, EditAnywhere, BlueprintReadWrite, Category = "Loading", meta = (ClampMin = "0.0", Units = "s"))
    float FadeOutTime = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    TSubclassOf<ULoadingWidget> LoadingWidgetClass;

    // Fires only after successful completion AND fade/removal are finished.
    UPROPERTY(BlueprintAssignable, Category = "Loading")
    FDebugLoadingFinishedEvent OnLoadingFinished;

    virtual void OnWorldEndPlay(UWorld& InWorld) override;
    virtual void Deinitialize() override;

protected:
    virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
    void UpdateLoading();
    void FinishLoading();

    UPROPERTY(Transient)
    TObjectPtr<ULoadingWidget> LoadingWidget;

    UPROPERTY(Transient)
    TWeakObjectPtr<APlayerController> OwningController;

    FTimerHandle UpdateTimer;
    bool bDebugSimulation = false;
    EDebugLoadingPhase Phase = EDebugLoadingPhase::Idle;
    float Progress = 0.0f;
    float ActiveLoadTime = 0.0f;
    float ActiveFadeTime = 0.0f;
    double LoadStartedAt = 0.0;
    double FadeStartedAt = 0.0;
};
