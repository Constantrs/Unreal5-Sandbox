// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CombatCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputComponent;
class UCombatStateController;
class UCombatCharacterMovementComponent;

UCLASS( ClassGroup=(CombatSandbox) )
class UNREAL5SANDBOX_API ACombatCharacter : public ACharacter
{
	GENERATED_BODY()

public:	
	ACombatCharacter(const FObjectInitializer& ObjectInitializer);
	
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void Jump() override;

	void Move(const FVector2D& MovementVector);

	void Look(const FVector2D& LookDirection);
	
	void Crouch();

	void Dash();

	bool IsMovementInputExist() const;
	
	UCombatStateController* GetCombatStateController() const
	{
		return StateController;
	}

	UCombatCharacterMovementComponent* GetCombatCharacterMovementComponent() const
	{
		return CombatCharacterMovement;
	}
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "CombatCharacter|Component")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "CombatCharacter|Component")
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleAnywhere, Category = "CombatCharacter|Component")
	UCombatStateController* StateController;

	UPROPERTY(VisibleAnywhere, Category = "CombatCharacter|Component")
	UCombatCharacterMovementComponent* CombatCharacterMovement;
	
	virtual void BeginPlay() override;
	
	virtual void UpdateCombatState(float DeltaTime);

	virtual void ResetInput();
private:
	bool IsCrouching = false;
	bool IsDashing = false;
};
