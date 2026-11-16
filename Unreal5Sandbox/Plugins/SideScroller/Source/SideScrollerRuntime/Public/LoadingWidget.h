#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadingWidget.generated.h"

class UProgressBar;
class UImage;
class UTexture2D;

/** Native UMG loading UI: a full-viewport background and a progress bar. */
UCLASS(BlueprintType, Blueprintable)
class SIDESCROLLERRUNTIME_API ULoadingWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Loading")
    void SetProgress(float InProgress);

    UFUNCTION(BlueprintPure, Category = "Loading")
    float GetProgress() const { return Progress; }

    UFUNCTION(BlueprintCallable, Category = "Loading|Appearance")
    void SetBackgroundTexture(UTexture2D* InTexture);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void SynchronizeProperties() override;

private:
    void UpdateBackground();

    /** Stretched to fill the viewport. Without a texture, shows an opaque black background. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loading|Appearance", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UTexture2D> BackgroundTexture;

    UPROPERTY(Transient)
    TObjectPtr<UImage> BackgroundImage;

    UPROPERTY(Transient)
    TObjectPtr<UProgressBar> ProgressBar;

    float Progress = 0.0f;
};
