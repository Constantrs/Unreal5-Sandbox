#include "SpherePawn.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputCoreTypes.h"
#include "UObject/ConstructorHelpers.h"

ASpherePawn::ASpherePawn()
{
    PrimaryActorTick.bCanEverTick = false;
    AutoPossessPlayer = EAutoReceiveInput::Player0;

    SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
    SetRootComponent(SphereCollision);
    
    SphereCollision->InitSphereRadius(50.0f);
    SphereCollision->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

    SphereMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereMesh"));
    SphereMesh->SetupAttachment(SphereCollision);
    SphereMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));
    Movement->SetUpdatedComponent(SphereCollision);
    Movement->MaxSpeed = 600.0f;
    Movement->Acceleration = 2400.0f;
    Movement->Deceleration = 3000.0f;
    Movement->SetPlaneConstraintNormal(FVector::UpVector);
    Movement->SetPlaneConstraintEnabled(true);

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(SphereCollision);
    SpringArm->TargetArmLength = 700.0f;
    SpringArm->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));
    SpringArm->bUsePawnControlRotation = false;
    SpringArm->bInheritPitch = false;
    SpringArm->bInheritYaw = false;
    SpringArm->bInheritRoll = false;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false;
}

void ASpherePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    check(PlayerInputComponent);

    PlayerInputComponent->BindAxisKey(EKeys::W, this, &ASpherePawn::MoveForward);
    PlayerInputComponent->BindAxisKey(EKeys::S, this, &ASpherePawn::MoveBackward);
    PlayerInputComponent->BindAxisKey(EKeys::A, this, &ASpherePawn::MoveLeft);
    PlayerInputComponent->BindAxisKey(EKeys::D, this, &ASpherePawn::MoveRight);
}

UPawnMovementComponent* ASpherePawn::GetMovementComponent() const
{
    return Movement.Get();
}

void ASpherePawn::MoveForward(float Value)
{
    AddMovementInput(FVector::ForwardVector, Value);
}

void ASpherePawn::MoveBackward(float Value)
{
    AddMovementInput(FVector::ForwardVector, -Value);
}

void ASpherePawn::MoveLeft(float Value)
{
    AddMovementInput(FVector::RightVector, -Value);
}

void ASpherePawn::MoveRight(float Value)
{
    AddMovementInput(FVector::RightVector, Value);
}
