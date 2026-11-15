#include "SideScrollerStageSelectGameMode.h"
#include "SideScrollerStageSelectController.h"

ASideScrollerStageSelectGameMode::ASideScrollerStageSelectGameMode()
{
    DefaultPawnClass = nullptr;
    PlayerControllerClass = ASideScrollerStageSelectController::StaticClass();
}
