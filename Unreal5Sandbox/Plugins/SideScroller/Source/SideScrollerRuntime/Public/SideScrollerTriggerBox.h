#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "SideScrollerObject.h"
#include "SideScrollerTriggerBox.generated.h"

class USideScrollerObjectComponent;

/** Box overlap Actor that participates in the shared object registry without requiring a mesh. */
UCLASS(Blueprintable, ClassGroup = (SideScroller))
class SIDESCROLLERRUNTIME_API ASideScrollerTriggerBox : public ATriggerBox, public ISideScrollerObject
{
	GENERATED_BODY()

public:
	ASideScrollerTriggerBox(const FObjectInitializer& ObjectInitializer);

	virtual ESideScrollerCategory GetCategory_Implementation() const override;
	virtual ESideScrollerHorizontalDirection GetHorizontalDirection_Implementation() const override;
	virtual float GetHorizontalDirectionValue_Implementation() const override;
	virtual void SetRootSkeletalMeshComponent_Implementation(USkeletalMeshComponent* Component) override;
	virtual USkeletalMeshComponent* GetRootSkeletalMeshComponent_Implementation() const override;

	/** Use as a guard in overlap gameplay logic. It does not defer/replay overlap events. */
	UFUNCTION(BlueprintPure, Category = "SideScroller|Trigger")
	bool IsStageReady() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SideScroller|Components")
	TObjectPtr<USideScrollerObjectComponent> ObjectComponent;
};