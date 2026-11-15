#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SideScrollerStageSelectGameMode.generated.h"

/** Stage selection needs a PlayerController but no playable Pawn. */
UCLASS()
class SIDESCROLLERRUNTIME_API ASideScrollerStageSelectGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ASideScrollerStageSelectGameMode();
};
