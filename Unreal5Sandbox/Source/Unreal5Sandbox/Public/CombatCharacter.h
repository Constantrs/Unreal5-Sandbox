// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "CombatCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UCombatStateController;
class UInputMappingContext;
class UInputAction;
class UEnhancedInputComponent;

UCLASS( ClassGroup=(CombatSandbox) )
class UNREAL5SANDBOX_API ACombatCharacter : public ACharacter
{
	GENERATED_BODY()

public:	
	ACombatCharacter();
	
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UCombatStateController* GetCombatStateController() const
	{
		return StateController;
	}
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditDefaultsOnly, Category = "CombatCharacter|Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "CombatCharacter|Input")
	TObjectPtr<UInputAction> InputAction_Movement;

	UPROPERTY(EditDefaultsOnly, Category = "CombatCharacter|Input")
	TObjectPtr<UInputAction> InputAction_Look;
	
	virtual void RegisterCombatInput();
	virtual void BindInputActions(UEnhancedInputComponent* EnhancedInputComponent);

	virtual  void UpdateCombatState();
	
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
private:
	bool InputInitialized = false;
	bool MoveInputValid = false;

	UPROPERTY(VisibleAnywhere, Category = "CombatCharacter|Component")
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "CombatCharacter|Component")
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleAnywhere, Category = "CombatCharacter|Component")
	UCombatStateController* StateController;
};
