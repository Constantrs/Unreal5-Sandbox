#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StageSetExportComponent.generated.h"

/** Opts an Actor into export. This metadata component is stripped when cooking. */
UCLASS(ClassGroup = (StageSet), meta = (BlueprintSpawnableComponent))
class STAGESETRUNTIME_API UStageSetExportComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UStageSetExportComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Set")
    FName RowName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Set")
    int32 Category = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Set")
    int32 Id = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Set")
    int32 AreaId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Set")
    int32 Param01 = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Set")
    int32 Param02 = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Set")
    int32 Param03 = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Set")
    int32 Param04 = 0;
};
