#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TimerManager.h"
#include "SideScrollerObjectTypes.h"
#include "SideScrollerGameMode.generated.h"

/** Gameplay entry point for the current single-player loading flow. */
UCLASS(Blueprintable, meta = (PrioritizeCategories = "Stage"))
class SIDESCROLLERRUNTIME_API ASideScrollerGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    ASideScrollerGameMode();

    virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

protected:
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stage")
    TArray<FSideScrollerObjectInfo> ObjectInfos;

    // Distinguishes direct map startup from an explicitly supplied empty list.
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Stage")
    bool bHasObjectInfoOptions = false;
    
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
private:
    void TryStartLoading();
};

