// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "CombatStateController.h"
#include "CombatAnimInstance.generated.h"

class ACombatCharacter;

UCLASS( ClassGroup=(CombatSandbox) )
class UNREAL5SANDBOX_API UCombatAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
	UPROPERTY(BlueprintReadOnly, Category = "CombatAnimInstance")
	ACombatCharacter* CombatCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "CombatAnimInstance|CharacterState")
	ECombatState CombatState;
};
