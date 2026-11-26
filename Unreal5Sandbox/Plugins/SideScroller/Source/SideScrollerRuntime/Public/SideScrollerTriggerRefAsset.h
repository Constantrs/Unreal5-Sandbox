#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SideScrollerTriggerRefAsset.generated.h"

USTRUCT(BlueprintType)
struct SIDESCROLLERRUNTIME_API FSideScrollerTriggerEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Id = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<AActor> TriggerClass;
};

UCLASS(BlueprintType, ClassGroup = (SideScroller))
class SIDESCROLLERRUNTIME_API USideScrollerTriggerRefAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trigger")
	TArray<FSideScrollerTriggerEntry> Triggers;

	bool ValidateCatalog(FString& OutError) const;
	const FSideScrollerTriggerEntry* FindTrigger(int32 Id) const;
};