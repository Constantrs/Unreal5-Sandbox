#include "ActorPlayerController.h"

#include "ActorPawn.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

void AActorPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (!IsLocalController()) return;

    // 入力基本設定
    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (auto* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            if (ensureMsgf(PlayerMappingContext, TEXT("Assign PlayerMappingContext in the controller Blueprint.")))
            {
                Subsystem->AddMappingContext(PlayerMappingContext, 0);
            }
        }
    }
    SetInputMode(FInputModeGameOnly());
    bShowMouseCursor = false;
    SetControlRotation(FRotator(-25.0f, GetControlRotation().Yaw, 0.0f));
}

void AActorPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 入力終了
    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (auto* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            if (PlayerMappingContext) Subsystem->RemoveMappingContext(PlayerMappingContext);
        }
    }
    Super::EndPlay(EndPlayReason);
}

void AActorPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    auto* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
    if (!ensureMsgf(EnhancedInputComponent, TEXT("Set Default Input Component Class to EnhancedInputComponent."))) return;

    if (ensure(MoveAction))
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AActorPlayerController::HandleMove);
    if (ensure(LookAction))
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AActorPlayerController::HandleLook);
    if (ensure(JumpAction))
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AActorPlayerController::HandleJump);
}

void AActorPlayerController::HandleMove(const FInputActionValue& Value)
{
    if (auto* playerPawn = Cast<AActorPawn>(GetPawn()))
    {
        playerPawn->Move(Value.Get<FVector2D>());   
    }
}

void AActorPlayerController::HandleLook(const FInputActionValue& Value)
{
    if (auto* playerPawn = Cast<AActorPawn>(GetPawn()))
    {
        auto inputValue = Value.Get<FVector2D>();
        if (InverseYAxis)
        {
            inputValue.Y *= -1.0f;
        }
        playerPawn->Look(inputValue);
    }
}

void AActorPlayerController::HandleJump(const FInputActionValue& Value)
{
    if (Value.Get<bool>())
    {
        if (auto* playerPawn = Cast<AActorPawn>(GetPawn()))
        {
            playerPawn->Jump();
        }
    }
}
