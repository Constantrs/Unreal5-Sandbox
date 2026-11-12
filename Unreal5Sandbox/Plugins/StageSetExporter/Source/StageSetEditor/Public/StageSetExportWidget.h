#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "Input/Reply.h"
#include "StageSetExportWidget.generated.h"

class AActor;
class UStageSetTable;

UENUM(BlueprintType)
enum class EStageSetExportMode : uint8
{
    Upsert UMETA(DisplayName = "Update matching rows and add new rows"),
    ReplaceAll UMETA(DisplayName = "Replace ALL table rows with this export")
};

/** Separate settings object keeps the tool's Details panel focused on export options. */
UCLASS(BlueprintType)
class STAGESETEDITOR_API UStageSetExportSettings : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, Category = "Export")
    TObjectPtr<UStageSetTable> TargetTable;

    UPROPERTY(EditAnywhere, Category = "Filter")
    TSubclassOf<AActor> ActorClass;

    UPROPERTY(EditAnywhere, Category = "Filter")
    FName RequiredActorTag = NAME_None;

    UPROPERTY(EditAnywhere, Category = "Filter")
    bool bOnlySelectedActors = false;

    UPROPERTY(EditAnywhere, Category = "Filter")
    bool bOnlyCurrentLevel = false;

    UPROPERTY(EditAnywhere, Category = "Export")
    EStageSetExportMode ExportMode = EStageSetExportMode::Upsert;

    UPROPERTY(EditAnywhere, Category = "Export")
    bool bSaveAfterExport = true;
};

/** Native UI: an empty Editor Utility Widget Blueprint can inherit this class. */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Stage Set Export Widget"))
class STAGESETEDITOR_API UStageSetExportWidget : public UEditorUtilityWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Stage Set Export")
    bool ValidateActors();

    UFUNCTION(BlueprintCallable, Category = "Stage Set Export")
    bool ExportActors();

    UFUNCTION(BlueprintCallable, Category = "Stage Set Export")
    bool AssignMissingRowNames();

    UFUNCTION(BlueprintCallable, Category = "Stage Set Export")
    bool SaveTargetTable();

    UPROPERTY(Transient, BlueprintReadOnly, Category = "Stage Set Export")
    TObjectPtr<UStageSetExportSettings> Settings;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    bool CollectActors(TArray<AActor*>& OutActors, FString& OutError) const;
    bool ValidateCollectedActors(const TArray<AActor*>& Actors, FString& OutError) const;
    void SetStatus(const FString& Message, bool bError = false);
    FText GetStatusText() const;
    FReply OnValidateClicked();
    FReply OnExportClicked();
    FReply OnAssignNamesClicked();
    FReply OnSaveClicked();

    FText StatusText;
};
