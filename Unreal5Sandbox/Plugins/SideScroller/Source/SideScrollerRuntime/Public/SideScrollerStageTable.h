#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SideScrollerStageTable.generated.h"

USTRUCT(BlueprintType)
struct SIDESCROLLERRUNTIME_API FSideScrollerStageRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
    FName StageName = NAME_None;

    // Prefer a full package name, e.g. /Game/SideScroller/Maps/L_Stage01.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stage")
    FString LevelName;
};

UCLASS(BlueprintType)
class SIDESCROLLERRUNTIME_API USideScrollerStageTable : public UDataTable
{
    GENERATED_BODY()

public:
    USideScrollerStageTable();
};
