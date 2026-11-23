#include "SideScrollerPawn.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "SideScrollerObjectComponent.h"

ASideScrollerPawn::ASideScrollerPawn()
{
    PrimaryActorTick.bCanEverTick = false;
    CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
    CapsuleComponent->InitCapsuleSize(42.0f, 96.0f);
    CapsuleComponent->SetCollisionProfileName(TEXT("Pawn"));
    SetRootComponent(CapsuleComponent);

    MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(CapsuleComponent);
    MeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
    ObjectComponent = CreateDefaultSubobject<USideScrollerObjectComponent>(TEXT("ObjectComponent"));
}

ESideScrollerCategory ASideScrollerPawn::GetCategory_Implementation() const
{
    return ObjectComponent->GetCategory();
}

ESideScrollerHorizontalDirection ASideScrollerPawn::GetHorizontalDirection_Implementation() const
{
    return ObjectComponent->GetHorizontalDirection();
}

float ASideScrollerPawn::GetHorizontalDirectionValue_Implementation() const
{
    return ObjectComponent->GetHorizontalDirectionValue();
}

void ASideScrollerPawn::SetRootSkeletalMeshComponent_Implementation(USkeletalMeshComponent* Component)
{
    ObjectComponent->SetRootSkeletalMeshComponent(Component);
}

USkeletalMeshComponent* ASideScrollerPawn::GetRootSkeletalMeshComponent_Implementation() const
{
    return ObjectComponent->GetRootSkeletalMeshComponent();
}

void ASideScrollerPawn::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    SetRootSkeletalMeshComponent_Implementation(MeshComponent);
}
