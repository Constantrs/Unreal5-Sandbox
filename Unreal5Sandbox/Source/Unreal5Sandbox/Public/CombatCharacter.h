// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "CombatCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UCombatStateController;

UCLASS( ClassGroup=(CombatSandbox) )
class UNREAL5SANDBOX_API ACombatCharacter : public ACharacter
{
	GENERATED_BODY()

public:	
	ACombatCharacter();
	
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Jump() override;

	bool IsLastInputExist() const;
	
	UCombatStateController* GetCombatStateController() const
	{
		return StateController;
	}
protected:
	virtual void BeginPlay() override;
	
	virtual void UpdateCombatState(float DeltaTime);
private:
	
	UPROPERTY(VisibleAnywhere, Category = "CombatCharacter|Component")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "CombatCharacter|Component")
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleAnywhere, Category = "CombatCharacter|Component")
	UCombatStateController* StateController;
};
