#include "SideScrollerTriggerBox.h"

#include "Engine/World.h"
#include "SideScrollerGameMode.h"
#include "SideScrollerObjectComponent.h"

ASideScrollerTriggerBox::ASideScrollerTriggerBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	ObjectComponent = CreateDefaultSubobject<USideScrollerObjectComponent>(TEXT("ObjectComponent"));
	ObjectComponent->SetCategory(ESideScrollerCategory::TGR);
}

ESideScrollerCategory ASideScrollerTriggerBox::GetCategory_Implementation() const
{
	return ObjectComponent->GetCategory();
}

ESideScrollerHorizontalDirection ASideScrollerTriggerBox::GetHorizontalDirection_Implementation() const
{
	return ObjectComponent->GetHorizontalDirection();
}

float ASideScrollerTriggerBox::GetHorizontalDirectionValue_Implementation() const
{
	return ObjectComponent->GetHorizontalDirectionValue();
}

void ASideScrollerTriggerBox::SetRootSkeletalMeshComponent_Implementation(USkeletalMeshComponent* Component)
{
	ObjectComponent->SetRootSkeletalMeshComponent(Component);
}

USkeletalMeshComponent* ASideScrollerTriggerBox::GetRootSkeletalMeshComponent_Implementation() const
{
	return ObjectComponent->GetRootSkeletalMeshComponent();
}

bool ASideScrollerTriggerBox::IsStageReady() const
{
	const ASideScrollerGameMode* Mode = GetWorld() ? GetWorld()->GetAuthGameMode<ASideScrollerGameMode>() : nullptr;
	return Mode && Mode->IsStageReady();
}