#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SideScrollerObject.h"
#include "SideScrollerPawn.generated.h"

class UCapsuleComponent;
class USkeletalMeshComponent;
class USideScrollerObjectComponent;

UCLASS(Blueprintable, ClassGroup = (SideScroller))
class SIDESCROLLERRUNTIME_API ASideScrollerPawn : public APawn, public ISideScrollerObject
{
    GENERATED_BODY()

public:
    ASideScrollerPawn();

    virtual ESideScrollerCategory GetCategory_Implementation() const override;
    virtual ESideScrollerHorizontalDirection GetHorizontalDirection_Implementation() const override;
    virtual float GetHorizontalDirectionValue_Implementation() const override;
    virtual void SetRootSkeletalMeshComponent_Implementation(USkeletalMeshComponent* Component) override;
    virtual USkeletalMeshComponent* GetRootSkeletalMeshComponent_Implementation() const override;
    virtual void PostInitializeComponents() override;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SideScroller|Components")
    TObjectPtr<UCapsuleComponent> CapsuleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SideScroller|Components")
    TObjectPtr<USkeletalMeshComponent> MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SideScroller|Components")
    TObjectPtr<USideScrollerObjectComponent> ObjectComponent;
};
