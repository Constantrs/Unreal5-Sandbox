#include "SideScrollerStageSelectController.h"

#include "Components/InputComponent.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/PackageName.h"
#include "SideScrollerStageSelectWidget.h"
#include "SideScrollerStageConfigAsset.h"

DEFINE_LOG_CATEGORY_STATIC(LogSideScrollerStageSelect, Log, All);

ASideScrollerStageSelectController::ASideScrollerStageSelectController()
{
    StageSelectWidgetClass = USideScrollerStageSelectWidget::StaticClass();
}

void ASideScrollerStageSelectController::BeginPlay()
{
    Super::BeginPlay();
    if (!IsLocalController()) return;
    if (GEngine)
        TravelFailureHandle = GEngine->OnTravelFailure().AddUObject(this, &ASideScrollerStageSelectController::HandleTravelFailure);

    StageSelectWidget = CreateWidget<USideScrollerStageSelectWidget>(this, StageSelectWidgetClass);
    if (!StageSelectWidget)
    {
        UE_LOG(LogSideScrollerStageSelect, Error, TEXT("Cannot create StageSelectWidget. Set its class in the Controller Blueprint."));
        return;
    }
    StageSelectWidget->SetStageConfigs(StageConfigs);
    StageSelectWidget->AddToViewport();
    // Keyboard input is handled exclusively by this Controller, not UMG focus/navigation.
    SetInputMode(FInputModeGameOnly());
    bShowMouseCursor = false;
}

void ASideScrollerStageSelectController::SetupInputComponent()
{
    Super::SetupInputComponent();
    if (!InputComponent) return;
    InputComponent->BindKey(EKeys::Up, IE_Pressed, this, &ASideScrollerStageSelectController::SelectPrevious);
    InputComponent->BindKey(EKeys::Down, IE_Pressed, this, &ASideScrollerStageSelectController::SelectNext);
    InputComponent->BindKey(EKeys::Up, IE_Repeat, this, &ASideScrollerStageSelectController::SelectPrevious);
    InputComponent->BindKey(EKeys::Down, IE_Repeat, this, &ASideScrollerStageSelectController::SelectNext);
    InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &ASideScrollerStageSelectController::ConfirmStage);
    InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ASideScrollerStageSelectController::ExitGame);
}

void ASideScrollerStageSelectController::SelectPrevious()
{
    if (!bOpeningLevel && StageSelectWidget) StageSelectWidget->MoveSelection(-1);
}

void ASideScrollerStageSelectController::SelectNext()
{
    if (!bOpeningLevel && StageSelectWidget) StageSelectWidget->MoveSelection(1);
}

void ASideScrollerStageSelectController::ConfirmStage()
{
    if (bOpeningLevel || !IsLocalController() || !StageSelectWidget)
    {
        return;
    }
    FSideScrollerStageConfig Selected;
    if (!StageSelectWidget->GetSelectedStage(Selected))
    {
        return;
    }
    const FString Level = Selected.Level.ToSoftObjectPath().GetLongPackageName();
    if (Level.IsEmpty() || (Level.StartsWith(TEXT("/")) && !FPackageName::DoesPackageExist(Level)))
    {
        StageSelectWidget->SetStatusMessage(FText::FromString(FString::Printf(TEXT("ステージ見つかりませんでした：%s，Levelとパッケージマップをチェックしてください。"), *Level)));
        return;
    }
    
    bOpeningLevel = true;
    UE_LOG(LogSideScrollerStageSelect, Display, TEXT("Opening stage %s: %s"), *Selected.StageId.ToString(), *Level);
    
    const FString Options = USideScrollerStageConfigAsset::BuildStageOptions(StageSelectWidget->GetSelectedStageId());
    UGameplayStatics::OpenLevel(this, FName(*Level), true, Options);
    //UGameplayStatics::OpenLevel(this, FName(*Level), true);
}

void ASideScrollerStageSelectController::ExitGame()
{
    if (!IsLocalController()) return;
    UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}

void ASideScrollerStageSelectController::HandleTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& Error)
{
    if (World != GetWorld()) return;
    bOpeningLevel = false;
    if (StageSelectWidget) StageSelectWidget->SetStatusMessage(FText::FromString(Error));
    UE_LOG(LogSideScrollerStageSelect, Error, TEXT("OpenLevel failed: %s"), *Error);
}

void ASideScrollerStageSelectController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (GEngine) GEngine->OnTravelFailure().Remove(TravelFailureHandle);
    if (StageSelectWidget) StageSelectWidget->RemoveFromParent();
    StageSelectWidget = nullptr;
    Super::EndPlay(EndPlayReason);
}
