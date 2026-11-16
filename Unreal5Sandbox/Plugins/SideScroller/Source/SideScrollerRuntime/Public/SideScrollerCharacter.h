#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SideScrollerObject.h"
#include "SideScrollerCharacter.generated.h"

class USideScrollerObjectComponent;

/** Uses ACharacter's existing capsule, skeletal mesh and movement components. */
UCLASS(Blueprintable)
class SIDESCROLLERRUNTIME_API ASideScrollerCharacter : public ACharacter, public ISideScrollerObject
{
    GENERATED_BODY()

public:
    ASideScrollerCharacter();

    virtual ESideScrollerCategory GetCategory_Implementation() const override;
    virtual ESideScrollerHorizontalDirection GetHorizontalDirection_Implementation() const override;
    virtual float GetHorizontalDirectionValue_Implementation() const override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SideScroller|Components")
    TObjectPtr<USideScrollerObjectComponent> ObjectComponent;
};
