#include "CombatCharacterMovementComponent.h"

#ifdef WITH_EDITOR
#include "DebugBlueprintFunctionLibrary.h"
#endif

UCombatCharacterMovementComponent::UCombatCharacterMovementComponent()
{
	NavAgentProps.bCanCrouch = true;
}

void UCombatCharacterMovementComponent::SprintPressed()
{
	
#ifdef WITH_EDITOR
	UDebugBlueprintFunctionLibrary::PrintScreenMessage(FColor::Cyan, "SprintPressed");
#endif
	bWantsToSprint = true;
}

void UCombatCharacterMovementComponent::SprintReleased()
{
#ifdef WITH_EDITOR
	UDebugBlueprintFunctionLibrary::PrintScreenMessage(FColor::Cyan, "SprintReleased");
#endif
	bWantsToSprint = false;
}

void UCombatCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation,
	const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	if (MovementMode == MOVE_Walking)
	{
		MaxWalkSpeed = (bWantsToSprint) ? SprintMaxSpeed : WalkMaxSpeed;
	}
}
