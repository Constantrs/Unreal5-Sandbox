#include "ActorPlayerController.h"

#include "ActorPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogDebugCamera, Log, All);

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

#if !UE_BUILD_SHIPPING
    FindDebugTopDownCamera();
#endif
}

void AActorPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DebugTopDownCamera.Reset();
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
    
#if !UE_BUILD_SHIPPING
    // Bind through UInputComponent; debug keys need no Enhanced Input assets.
    if (InputComponent)
    {
        InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AActorPlayerController::DebugUsePlayerCamera);
        InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AActorPlayerController::DebugUseTopDownCamera);
    }
#endif
    
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

void AActorPlayerController::FindDebugTopDownCamera()
{
#if !UE_BUILD_SHIPPING
    DebugTopDownCamera.Reset();
    if (!IsLocalController() || DebugTopDownCameraTag.IsNone()) return;

    TArray<AActor*> Candidates;
    UGameplayStatics::GetAllActorsWithTag(this, DebugTopDownCameraTag, Candidates);
    for (AActor* Candidate : Candidates)
    {
        if (!IsValid(Candidate)) continue;
        TArray<UCameraComponent*> Cameras;
        Candidate->GetComponents<UCameraComponent>(Cameras);
        for (UCameraComponent* Camera : Cameras)
        {
            if (IsValid(Camera) && Camera->IsActive())
            {
                if (DebugTopDownCamera.IsValid())
                {
                    UE_LOG(LogDebugCamera, Warning, TEXT("Multiple active camera actors use tag '%s'; keeping %s. Use a unique tag."),
                        *DebugTopDownCameraTag.ToString(), *DebugTopDownCamera->GetName());
                    return;
                }
                DebugTopDownCamera = Candidate;
                break;
            }
        }
    }
#endif
}

void AActorPlayerController::DebugUsePlayerCamera()
{
#if !UE_BUILD_SHIPPING
    if (!IsLocalController()) return;
    if (APawn* ControlledPawn = GetPawn())
    {
        SetViewTargetWithBlend(ControlledPawn, DebugCameraBlendTime);
    }
#endif
}

void AActorPlayerController::DebugUseTopDownCamera()
{
#if !UE_BUILD_SHIPPING
    if (!IsLocalController()) return;
    // Retry only when necessary, e.g. a streamed camera arrived after BeginPlay.
    if (!DebugTopDownCamera.IsValid()) FindDebugTopDownCamera();
    if (AActor* CameraActor = DebugTopDownCamera.Get())
    {
        SetViewTargetWithBlend(CameraActor, DebugCameraBlendTime);
    }
    else
    {
        UE_LOG(LogDebugCamera, Warning, TEXT("No actor with an active CameraComponent and tag '%s' was found."),
            *DebugTopDownCameraTag.ToString());
    }
#endif
}
