#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SideScrollerObjectTypes.h"
#include "SideScrollerStageTable.generated.h"

USTRUCT(BlueprintType)
struct SIDESCROLLERRUNTIME_API FSideScrollerStageRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName StageName = NAME_None;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LevelName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FSideScrollerObjectInfo> ObjectInfos;
};

UCLASS(BlueprintType)
class SIDESCROLLERRUNTIME_API USideScrollerStageTable : public UDataTable
{
    GENERATED_BODY()

public:
    USideScrollerStageTable();
};
