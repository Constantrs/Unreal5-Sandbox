// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatAnimInstance.h"
#include "CombatCharacter.h"

void UCombatAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	if (const auto combatCharacter = Cast<ACombatCharacter>(TryGetPawnOwner()))
	{
		CombatCharacter = combatCharacter;
	}
}

void UCombatAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (IsValid(CombatCharacter))
	{
		if (auto combatStateController = CombatCharacter->GetCombatStateController())
		{
			CombatState = combatStateController->GetCombatState();
		}
	}
}
