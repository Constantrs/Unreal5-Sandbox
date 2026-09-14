// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatPlayerController.h"

#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "CombatCharacter.h"
#include "CombatCharacterMovementComponent.h"

ACombatPlayerController::ACombatPlayerController()
{
	bReplicates = true;
}

void ACombatPlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetupInputController();
}

void ACombatPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	BindCombatInputActions();
}

#pragma region CharacterActions

void ACombatPlayerController::CharacterMove(const FInputActionValue& Value)
{
	const FVector2D movementVector = Value.Get<FVector2D>();
	
	if(auto controlledPawn = GetPawn<APawn>())
	{
		if (auto controlledCharacter = Cast<ACombatCharacter>(controlledPawn))
		{
			controlledCharacter->Move(movementVector);
		}
	}
}

void ACombatPlayerController::CharacterLook(const FInputActionValue& Value)
{
	const FVector2D lookAxisVector = Value.Get<FVector2D>();

	if(auto controlledPawn = GetPawn<APawn>())
	{
		if (auto controlledCharacter = Cast<ACombatCharacter>(controlledPawn))
		{
			controlledCharacter->Look(lookAxisVector);
		}
	}
}

void ACombatPlayerController::CharacterJump()
{
	if(auto controlledPawn = GetPawn<APawn>())
	{
		if (auto controlledCharacter = Cast<ACombatCharacter>(controlledPawn))
		{
			controlledCharacter->Jump();
		}
	}
}

void ACombatPlayerController::CharacterCrouch()
{
	if(auto controlledPawn = GetPawn<APawn>())
	{
		if (auto controlledCharacter = Cast<ACombatCharacter>(controlledPawn))
		{
			controlledCharacter->Crouch();
		}
	}
}

void ACombatPlayerController::CharacterDash()
{
	if(auto controlledPawn = GetPawn<APawn>())
	{
		if (auto controlledCharacter = Cast<ACombatCharacter>(controlledPawn))
		{
			controlledCharacter->Dash();
		}
	}
}

#pragma endregion CharacterActions

void ACombatPlayerController::SprintPressed()
{
	if(auto controlledPawn = GetPawn<APawn>())
	{
		if (auto controlledCharacter = Cast<ACombatCharacter>(controlledPawn))
		{
			if (auto combatMovement = controlledCharacter->GetCombatCharacterMovementComponent())
			{
				combatMovement->SprintPressed();
			}
		}
	}
}

void ACombatPlayerController::SprintReleased()
{
	if(auto controlledPawn = GetPawn<APawn>())
	{
		if (auto controlledCharacter = Cast<ACombatCharacter>(controlledPawn))
		{
			if (auto combatMovement = controlledCharacter->GetCombatCharacterMovementComponent())
			{
				combatMovement->SprintReleased();
			}
		}
	}
}

void ACombatPlayerController::SetupInputController()
{
	check(InputMappingContext);

	UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	check(subsystem);

	subsystem->AddMappingContext(InputMappingContext, 0);
	UE_LOG(LogTemp, Log, TEXT("<ACombatPlayerController::SetupInputController> Setup Controller Input Succeeded!"));
	
	bShowMouseCursor = false;
	DefaultMouseCursor = EMouseCursor::Default;
}	

void ACombatPlayerController::BindCombatInputActions()
{
	if (UEnhancedInputComponent* enhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		enhancedInputComponent->BindAction(InputAction_Movement, ETriggerEvent::Triggered, this, &ACombatPlayerController::CharacterMove);
		enhancedInputComponent->BindAction(InputAction_Look, ETriggerEvent::Triggered, this, &ACombatPlayerController::CharacterLook);
		enhancedInputComponent->BindAction(InputAction_Jump, ETriggerEvent::Triggered, this, &ACombatPlayerController::CharacterJump);
		enhancedInputComponent->BindAction(InputAction_Crouch, ETriggerEvent::Triggered, this, &ACombatPlayerController::CharacterCrouch);
		enhancedInputComponent->BindAction(InputAction_Dash, ETriggerEvent::Triggered, this, &ACombatPlayerController::CharacterDash);

		enhancedInputComponent->BindAction(InputAction_Sprint, ETriggerEvent::Started, this, &ACombatPlayerController::SprintPressed);
		enhancedInputComponent->BindAction(InputAction_Sprint, ETriggerEvent::Completed, this, &ACombatPlayerController::SprintReleased);
		UE_LOG(LogTemp, Log, TEXT("<ACombatPlayerController::BindCombatInputActions> Bind Input Actions Succeeded!"));
	}
}


