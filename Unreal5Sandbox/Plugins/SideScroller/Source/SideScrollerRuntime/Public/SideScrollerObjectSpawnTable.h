#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "SideScrollerObjectTypes.h"
#include "SideScrollerObjectSpawnTable.generated.h"

USTRUCT(BlueprintType)
struct SIDESCROLLERRUNTIME_API FSideScrollerObjectSpawnRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FSideScrollerObjectInfo ObjectInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FSideScrollerObjectParam ObjectParam;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FTransform Transform = FTransform::Identity;
};
