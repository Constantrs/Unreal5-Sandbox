#pragma once

#include "CoreMinimal.h"
#include "SideScrollerObjectTypes.generated.h"

UENUM(BlueprintType)
enum class ESideScrollerCategory : uint8
{
    Undefined,
    Player,
    Enemy,
    Gimmick
};

UENUM(BlueprintType)
enum class ESideScrollerHorizontalDirection : uint8
{
    Left,
    Right,
    Spline
};
