#pragma once

#include "CoreMinimal.h"
#include "SideScrollerObjectTypes.generated.h"

UENUM(BlueprintType)
enum class ESideScrollerCategory : uint8
{
    Undefined = 0   UMETA(Hidden),
    PL = 1          UMETA(DisplayName="Player"),
    EM = 2          UMETA(DisplayName="Enemy"),
    GMK = 3         UMETA(DisplayName="Gimmick"),
    TGR = 4         UMETA(DisplayName = "Trigger")
};

UENUM(BlueprintType)
enum class ESideScrollerHorizontalDirection : uint8
{
    Left,
    Right,
    Spline
};

USTRUCT(BlueprintType)
struct SIDESCROLLERRUNTIME_API FSideScrollerObjectInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESideScrollerCategory Category = ESideScrollerCategory::Undefined;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Id = 0;
};

USTRUCT(BlueprintType)
struct SIDESCROLLERRUNTIME_API FSideScrollerObjectParam
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SideScroller|Object")
    int32 Type = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SideScroller|Object")
    float Param1 = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SideScroller|Object")
    float Param2 = 0.0f;
};