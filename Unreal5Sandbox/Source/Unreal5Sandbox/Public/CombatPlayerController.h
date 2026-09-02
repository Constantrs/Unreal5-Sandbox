// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CombatPlayerController.generated.h"

struct FInputActionValue;
class UInputMappingContext;
class UInputAction;

UCLASS( ClassGroup=(CombatSandbox) )
class UNREAL5SANDBOX_API ACombatPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACombatPlayerController();

protected:
	virtual void BeginPlay() override;

	virtual  void SetupInputComponent() override;

	virtual void CharacterMove(const FInputActionValue& Value);
	
	virtual void CharacterLook(const FInputActionValue& Value);

	virtual void CharacterJump();
private:
	UPROPERTY(EditDefaultsOnly, Category = "CombatPlayerController|Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "CombatPlayerController|Input")
	TObjectPtr<UInputAction> InputAction_Movement;

	UPROPERTY(EditDefaultsOnly, Category = "CombatPlayerController|Input")
	TObjectPtr<UInputAction> InputAction_Look;

	UPROPERTY(EditDefaultsOnly, Category = "CombatPlayerController|Input")
	TObjectPtr<UInputAction> InputAction_Jump;

	void SetupInputController();

	void BindCombatInputActions();
};
