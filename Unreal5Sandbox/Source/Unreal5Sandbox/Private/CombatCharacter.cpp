// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "CombatStateController.h"

#include "Components/InputComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"

ACombatCharacter::ACombatCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	if (auto charaMovementComponent = GetCharacterMovement())
	{
		charaMovementComponent->bOrientRotationToMovement = true;
		charaMovementComponent->RotationRate = FRotator(0, 540.0f, 0);
	}
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(FName("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(FName("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	StateController = CreateDefaultSubobject<UCombatStateController>(FName("CombatStateController"));
}

void ACombatCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateCombatState();
}

void ACombatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* enhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		UE_LOG(LogTemp, Log, TEXT("<ACombatCharacter::SetupPlayerInputComponent> BindInputActions"));
		
		BindInputActions(enhancedInputComponent);
	}
}

void ACombatCharacter::BeginPlay()
{
	Super::BeginPlay();

	RegisterCombatInput();
}

void ACombatCharacter::RegisterCombatInput()
{
	if (!InputMappingContext)
	{
		return;
	}

	if (const APlayerController* playerController = GetController<APlayerController>())
	{
		if (const ULocalPlayer* localPlayer = playerController->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* subsystem = localPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				subsystem->AddMappingContext(InputMappingContext, 0);
				InputInitialized = true;
				UE_LOG(LogTemp, Log, TEXT("<ACombatCharacter::RegisterCombatInput> Register Input Succeeded"));
			}
		}
	}
}

void ACombatCharacter::BindInputActions(UEnhancedInputComponent* EnhancedInputComponent)
{
	EnhancedInputComponent->BindAction(InputAction_Movement, ETriggerEvent::Triggered, this, &ACombatCharacter::Move);
	EnhancedInputComponent->BindAction(InputAction_Look, ETriggerEvent::Triggered, this, &ACombatCharacter::Look);
}

void ACombatCharacter::UpdateCombatState()
{
	if (IsValid(StateController))
	{
		StateController->SetCombatState(MoveInputValid ? ECombatState::State_Run : ECombatState::State_Idle);
	}
	MoveInputValid = false;
}

void ACombatCharacter::Move(const FInputActionValue& Value)
{
	if (!InputInitialized)
	{
		return;
	}
	
	auto controller = GetController();
	if (!IsValid(controller))
	{
		return;
	}

	const FVector2D movementVector = Value.Get<FVector2D>();
	const FRotator rotation = controller->GetControlRotation();
	const FRotator yawRotation(0.0f, rotation.Yaw, 0.0f);

	const FVector forwardVector = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);
	const FVector rightVector = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);

	UE_LOG(LogTemp, Log, TEXT("<ACombatCharacter::RegisterCombatInput> Move"));
	
	AddMovementInput(forwardVector, movementVector.Y);
	AddMovementInput(rightVector, movementVector.X);

	MoveInputValid = true;
}

void ACombatCharacter::Look(const FInputActionValue& Value)
{
	if (!InputInitialized)
	{
		return;
	}
	
	auto controller = GetController();
	if (!IsValid(controller))
	{
		return;
	}

	const FVector2D lookAxisVector = Value.Get<FVector2D>();

	AddControllerPitchInput(lookAxisVector.Y);
	AddControllerYawInput(lookAxisVector.X);
}
