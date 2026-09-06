// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatCameraManager.h"

#include "CombatCharacter.h"
#include "CombatCharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"


void ACombatCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	Super::UpdateViewTarget(OutVT, DeltaTime);

	if (auto combatCharacter = Cast<ACombatCharacter>(GetOwningPlayerController()->GetPawn()))
	{
		if (auto combatCharacterMovementComponent = combatCharacter->GetCombatCharacterMovementComponent())
		{
			float height = combatCharacterMovementComponent->GetCrouchedHalfHeight() - combatCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			float t = FMath::Clamp(CrouchBlendTime / CrouchBlendDuration, 0.0f, 1.0f);
			FVector TargetCrouchOffset = FVector(0, 0, height);
			FVector Offset = FMath::Lerp(FVector::ZeroVector, TargetCrouchOffset, t);

			if (combatCharacterMovementComponent->IsCrouching())
			{
				CrouchBlendTime =  FMath::Clamp(CrouchBlendTime + DeltaTime, 0.0f, CrouchBlendDuration);
				Offset -= TargetCrouchOffset;
			}
			else
			{
				CrouchBlendTime =  FMath::Clamp(CrouchBlendTime - DeltaTime, 0.0f, CrouchBlendDuration);
			}

			if (combatCharacterMovementComponent->IsMovingOnGround())
			{
				OutVT.POV.Location += Offset;
			}
		}
	}
}
