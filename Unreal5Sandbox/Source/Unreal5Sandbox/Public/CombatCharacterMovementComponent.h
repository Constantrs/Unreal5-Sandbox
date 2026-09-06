// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CombatCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS( ClassGroup=(CombatSandbox) )
class UNREAL5SANDBOX_API UCombatCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	UCombatCharacterMovementComponent();

	UPROPERTY(EditDefaultsOnly)
	float WalkMaxSpeed;

	UPROPERTY(EditDefaultsOnly)
	float SprintMaxSpeed;
	
	UFUNCTION(BlueprintCallable)
	void SprintPressed();

	UFUNCTION(BlueprintCallable)
	void SprintReleased();

protected:
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

private:
	bool bWantsToSprint;
};
