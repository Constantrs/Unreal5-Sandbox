#include "SideScrollerObjectComponent.h"

#include "Engine/World.h"
#include "SideScrollerGameState.h"

USideScrollerObjectComponent::USideScrollerObjectComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

float USideScrollerObjectComponent::GetHorizontalDirectionValue() const
{
    switch (HorizontalDirection)
    {
    case ESideScrollerHorizontalDirection::Left: return -1.0f;
    case ESideScrollerHorizontalDirection::Right: return 1.0f;
    case ESideScrollerHorizontalDirection::Spline: return 1.0f; // Future spline direction calculation.
    default: return 1.0f;
    }
}

void USideScrollerObjectComponent::SetCategory(ESideScrollerCategory InCategory)
{
    Category = InCategory;
    if (HasBegunPlay()) RegisterWithGameState();
}

bool USideScrollerObjectComponent::RegisterWithGameState()
{
    if (ASideScrollerGameState* Previous = RegisteredGameState.Get()) Previous->UnregisterObject(GetOwner());
    RegisteredGameState.Reset();
    ASideScrollerGameState* State = GetWorld() ? GetWorld()->GetGameState<ASideScrollerGameState>() : nullptr;
    if (!State || !State->RegisterObject(GetOwner())) return false;
    RegisteredGameState = State;
    return true;
}

void USideScrollerObjectComponent::BeginPlay()
{
    Super::BeginPlay();
    RegisterWithGameState();
}

void USideScrollerObjectComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ASideScrollerGameState* State = RegisteredGameState.Get()) State->UnregisterObject(GetOwner());
    RegisteredGameState.Reset();
    Super::EndPlay(EndPlayReason);
}
