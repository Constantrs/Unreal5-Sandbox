#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineBaseTypes.h"
#include "GameFramework/PlayerController.h"
#include "SideScrollerStageSelectController.generated.h"

class UDataTable;
class USideScrollerStageSelectWidget;

UCLASS(BlueprintType, Blueprintable)
class SIDESCROLLERRUNTIME_API ASideScrollerStageSelectController : public APlayerController
{
    GENERATED_BODY()

public:
    ASideScrollerStageSelectController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stage Select", meta = (RequiredAssetDataTags = "RowStructure=/Script/SideScrollerRuntime.SideScrollerStageRow"))
    TObjectPtr<UDataTable> StageTable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stage Select")
    TSubclassOf<USideScrollerStageSelectWidget> StageSelectWidgetClass;

private:
    void SelectPrevious();
    void SelectNext();
    void ConfirmStage();
    void ExitGame();
    void HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& Error);

    UPROPERTY(Transient)
    TObjectPtr<USideScrollerStageSelectWidget> StageSelectWidget;

    bool bOpeningLevel = false;
    FDelegateHandle TravelFailureHandle;
};
