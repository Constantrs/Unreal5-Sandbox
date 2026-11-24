#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SideScrollerStageConfigAsset.h"
#include "SideScrollerStageSelectWidget.generated.h"

class UTextBlock;
class UVerticalBox;

/** Native UMG widget. Owns a VerticalBox with exactly 20 reusable row widgets. */
UCLASS(BlueprintType, Blueprintable)
class SIDESCROLLERRUNTIME_API USideScrollerStageSelectWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    static constexpr int32 StagesPerPage = 20;

    UFUNCTION(BlueprintCallable, Category = "Stage Select")
    bool SetStageConfigs(USideScrollerStageConfigAsset* InConfigs);

    UFUNCTION(BlueprintCallable, Category = "Stage Select")
    void MoveSelection(int32 Direction);

    UFUNCTION(BlueprintPure, Category = "Stage Select")
    bool GetSelectedStage(FSideScrollerStageConfig& OutStage) const;

    UFUNCTION(BlueprintPure, Category = "Stage Select")
    FName GetSelectedStageId() const;

    UFUNCTION(BlueprintPure, Category = "Stage Select")
    int32 GetSelectedIndex() const { return SelectedIndex; }
    
    UFUNCTION(BlueprintPure, Category = "Stage Select")
    int32 GetStageCount() const { return Stages.Num(); }

    UFUNCTION(BlueprintPure, Category = "Stage Select")
    int32 GetPageCount() const;

    UFUNCTION(BlueprintPure, Category = "Stage Select")
    int32 GetCurrentPageIndex() const;

    void SetStatusMessage(const FText& Message);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    void BuildWidgetTree();
    void RefreshPage();

    UPROPERTY(Transient)
    TObjectPtr<USideScrollerStageConfigAsset> StageConfigs;

    UPROPERTY(Transient)
    TArray<FSideScrollerStageConfig> Stages;

    UPROPERTY(Transient)
    TArray<FName> StageIds;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> StageList;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UTextBlock>> StageLabels;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> PageLabel;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> StatusLabel;

    int32 SelectedIndex = INDEX_NONE;
    FText StatusMessage;
};
