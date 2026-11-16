#include "SideScrollerCharacter.h"

#include "SideScrollerObjectComponent.h"

ASideScrollerCharacter::ASideScrollerCharacter()
{
    ObjectComponent = CreateDefaultSubobject<USideScrollerObjectComponent>(TEXT("ObjectComponent"));
}

ESideScrollerCategory ASideScrollerCharacter::GetCategory_Implementation() const
{
    return ObjectComponent->GetCategory();
}

ESideScrollerHorizontalDirection ASideScrollerCharacter::GetHorizontalDirection_Implementation() const
{
    return ObjectComponent->GetHorizontalDirection();
}

float ASideScrollerCharacter::GetHorizontalDirectionValue_Implementation() const
{
    return ObjectComponent->GetHorizontalDirectionValue();
}
