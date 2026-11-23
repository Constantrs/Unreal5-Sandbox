#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SideScrollerObjectTypes.h"
#include "SideScrollerEnemySpawnRow.generated.h"

USTRUCT(BlueprintType)
struct SIDESCROLLERRUNTIME_API FSideScrollerEnemySpawnRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FSideScrollerObjectInfo ObjectInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FTransform Transform = FTransform::Identity;
};