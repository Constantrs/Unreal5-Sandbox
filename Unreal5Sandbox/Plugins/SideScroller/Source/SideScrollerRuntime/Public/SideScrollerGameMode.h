#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TimerManager.h"
#include "SideScrollerGameMode.generated.h"

/** Gameplay entry point for the current single-player loading flow. */
UCLASS(Blueprintable)
class SIDESCROLLERRUNTIME_API ASideScrollerGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    ASideScrollerGameMode();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    void TryStartLoading();
};
