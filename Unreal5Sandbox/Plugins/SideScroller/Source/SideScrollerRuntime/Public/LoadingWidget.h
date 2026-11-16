#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingWidget.generated.h"

class UProgressBar;

/** Minimal native UMG loading UI: a single progress bar, no resource loading logic. */
UCLASS(BlueprintType, Blueprintable)
class SIDESCROLLERRUNTIME_API ULoadingWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Loading")
    void SetProgress(float InProgress);

    UFUNCTION(BlueprintPure, Category = "Loading")
    float GetProgress() const { return Progress; }

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    UPROPERTY(Transient)
    TObjectPtr<UProgressBar> ProgressBar;

    float Progress = 0.0f;
};
