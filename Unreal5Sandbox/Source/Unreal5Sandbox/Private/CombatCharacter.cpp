// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "CombatStateController.h"

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

	UpdateCombatState(DeltaTime);
}

void ACombatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACombatCharacter::Jump()
{
	Super::Jump();

	auto currentState = StateController->GetCombatState();
	if (currentState > ECombatState::StateMaxIndex_OnGround)
	{
		if (auto charaMovementComponent = GetCharacterMovement())
		{
			if (!charaMovementComponent->IsFalling())
			{
				StateController->SetCombatState(ECombatState::State_Jump);
			}
		}	
	}
}

bool ACombatCharacter::IsLastInputExist() const
{
	const FVector lastInputVector = GetLastMovementInputVector();
	if (lastInputVector.SquaredLength() > 0)
	{
		return true;
	}
	return false;
}

void ACombatCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ACombatCharacter::UpdateCombatState(float DeltaTime)
{
	if (IsValid(StateController))
	{
		if (auto charaMovementComponent = GetCharacterMovement())
		{
			auto currentState = StateController->GetCombatState();

			// Ground to Air
			if (currentState <= ECombatState::StateMaxIndex_OnGround)
			{
				// TODO: check Fall
				StateController->SetCombatState(IsLastInputExist() ? ECombatState::State_Run : ECombatState::State_Idle);
			}
			// Air to Ground
			else if (currentState > ECombatState::StateMaxIndex_OnGround && currentState <= ECombatState::StateMaxIndex_OnAir)
			{
				if (!charaMovementComponent->IsFalling())
				{
					StateController->SetCombatState(ECombatState::State_Land);
				}
			}
		}
	}
}