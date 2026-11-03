#include "CombatCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "CombatStateController.h"
#include "CombatAnimInstance.h"
#include "CombatCharacterMovementComponent.h"

#ifdef WITH_EDITOR
#include "DebugBlueprintFunctionLibrary.h"
#endif

ACombatCharacter::ACombatCharacter(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer.SetDefaultSubobjectClass<UCombatCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(FName("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(FName("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	StateController = CreateDefaultSubobject<UCombatStateController>(FName("CombatStateController"));

	CombatCharacterMovement = Cast<UCombatCharacterMovementComponent>(GetCharacterMovement());
}

void ACombatCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (IsValid(CombatCharacterMovement))
	{
		if (IsCrouching != CombatCharacterMovement->bWantsToCrouch)
		{
			CombatCharacterMovement->bWantsToCrouch = IsCrouching;
		}
	}
	
	UpdateCombatState(DeltaTime);

	ResetInput();
}

void ACombatCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ACombatCharacter::Jump()
{
	if (IsValid(CombatCharacterMovement))
	{
		auto currentState = StateController->GetCombatState();
		if (CombatCharacterMovement->IsMovingOnGround() && currentState <= ECombatState::StateMaxIndex_OnGround)
		{
			StateController->SetCombatState(ECombatState::State_Jump);
			Super::Jump();
			
		}
	}	
}

void ACombatCharacter::Move(const FVector2D& MovementVector)
{
	if (IsValid(CombatCharacterMovement))
	{
		auto currentState = StateController->GetCombatState();
		if (CombatCharacterMovement->IsMovingOnGround() &&
			(currentState == ECombatState::State_Idle || currentState == ECombatState::State_Run))
		{
			const FRotator rotation = GetControlRotation();
			const FRotator yawRotation(0.0f, rotation.Yaw, 0.0f);

			const FVector forwardVector = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::X);
			const FVector rightVector = FRotationMatrix(yawRotation).GetUnitAxis(EAxis::Y);
			
			AddMovementInput(forwardVector, MovementVector.Y);
			AddMovementInput(rightVector, MovementVector.X);
			StateController->SetCombatState(ECombatState::State_Run, false);
		}
	}
}

void ACombatCharacter::Look(const FVector2D& LookDirection)
{
	AddControllerPitchInput(LookDirection.Y);
	AddControllerYawInput(LookDirection.X);
}

void ACombatCharacter::Crouch()
{
	if (IsValid(CombatCharacterMovement))
	{
		auto currentState = StateController->GetCombatState();
		if (CombatCharacterMovement->IsMovingOnGround() && currentState <= ECombatState::StateMaxIndex_OnGround)
		{
			IsCrouching = true;
		}
	}
}

void ACombatCharacter::Dash()
{
	if (IsDashing)
	{
		return;
	}
	
	if (!CombatCharacterMovement->IsFalling())
	{
		auto animInstance = Cast<UCombatAnimInstance>(GetMesh()->GetAnimInstance());
		if (IsValid(DashAnim) && animInstance)
		{
			animInstance->Montage_Play(DashAnim);
			FOnMontageEnded MontageEndedDelegate;
			MontageEndedDelegate.BindUObject(this, &ACombatCharacter::OnDashMontageEnded);
			animInstance->Montage_SetEndDelegate(MontageEndedDelegate, DashAnim);
			
			StateController->SetCombatState(ECombatState::State_Dash);
			IsDashing = true;	
		}
	}
	else
	{
		StateController->SetCombatState(ECombatState::State_AirDash);
		IsDashing = true;
	}
}

bool ACombatCharacter::IsMovementInputExist() const
{
	return !FMath::IsNearlyZero(GetLastMovementInputVector().SquaredLength());
}

void ACombatCharacter::OnDashMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
#ifdef WITH_EDITOR
	UDebugBlueprintFunctionLibrary::PrintScreenMessage(FColor::Green, FString("OnDashMontageEnded"));
#endif
	IsDashing = false;
}

void ACombatCharacter::BeginPlay()
{
	Super::BeginPlay();

	//BindAnimEndDelegate();
}

void ACombatCharacter::UpdateCombatState(float DeltaTime)
{
	if (IsValid(StateController))
	{
		if (IsValid(CombatCharacterMovement))
		{
			auto currentState = StateController->GetCombatState();

			// Ground
			if (currentState <= ECombatState::StateMaxIndex_OnGround)
			{
				if (currentState == ECombatState::State_Run)
				{
					// Run to Idle
					if (!IsMovementInputExist())
					{
						StateController->SetCombatState(ECombatState::State_Idle);
					}
				}
				else if (currentState == ECombatState::State_Dash)
				{
					// Dash to Idie
					if (!IsDashing)
					{
						StateController->SetCombatState(ECombatState::State_Idle);
					}
				}
				else if (currentState == ECombatState::State_Land)
				{
					// Land To Idle or Run
					StateController->SetCombatState( !IsMovementInputExist() ? ECombatState::State_Idle : ECombatState::State_Run );
				}

			}
			// Air
			else if (currentState > ECombatState::StateMaxIndex_OnGround && currentState <= ECombatState::StateMaxIndex_OnAir)
			{
				if (!CombatCharacterMovement->IsFalling())
				{
					// Air to Ground
					StateController->SetCombatState(ECombatState::State_Land);
				}
			}
		}
	}
}

void ACombatCharacter::ResetInput()
{
	IsCrouching = false;
}

void ACombatCharacter::BindAnimEndDelegate()
{
	wpCombatAnimInstance = Cast<UCombatAnimInstance>(GetMesh()->GetAnimInstance());
	auto animInstance = wpCombatAnimInstance.Get();
	if (IsValid(animInstance))
	{
		if (IsValid(DashAnim))
		{
			FOnMontageEnded MontageEndedDelegate;
			MontageEndedDelegate.BindUObject(this, &ACombatCharacter::OnDashMontageEnded);
			animInstance->Montage_SetEndDelegate(MontageEndedDelegate, DashAnim);
		}
	}
}
