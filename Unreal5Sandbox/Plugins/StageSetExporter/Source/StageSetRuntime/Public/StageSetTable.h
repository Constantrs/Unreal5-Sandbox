#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "StageSetTable.generated.h"

/** RowName is the DataTable's native key, not a second member in the row. */
USTRUCT(BlueprintType)
struct STAGESETRUNTIME_API FStageSetRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Set")
    int32 Category = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Set")
    int32 Id = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Set")
    int32 AreaId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Set")
    FTransform Transform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Set")
    int32 Param01 = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Set")
    int32 Param02 = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Set")
    int32 Param03 = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage Set")
    int32 Param04 = 0;
};

UCLASS(BlueprintType)
class STAGESETRUNTIME_API UStageSetTable : public UDataTable
{
    GENERATED_BODY()

public:
    UStageSetTable();
};
