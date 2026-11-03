#include "ActorPawn.h"

#include "Camera/CameraComponent.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "SubMeshControlComponent.h"
#include "SubMeshDataAsset.h"

AActorPawn::AActorPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    bUseControllerRotationYaw = false;

    Collision = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleCollision"));
    SetRootComponent(Collision);
    Collision->InitCapsuleSize( CapsuleRadius, CapsuleHeight );
    Collision->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

    // サブメッシュ動的に生成用コンポーネント
    SubMeshControlComponent = CreateDefaultSubobject<USubMeshControlComponent>(TEXT("SubMeshControlComponent"));
    
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(Collision);
    SpringArm->TargetArmLength = 650.0f;
    SpringArm->bUsePawnControlRotation = true;
    
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false;
}

void AActorPawn::Move(const FVector2D& Input)
{
    if (!Controller) return;

    // Use camera/control yaw only: looking up must not make the pawn fly.
    const FRotator Yaw(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
    AddMovementInput(Yaw.RotateVector(FVector::ForwardVector), Input.Y);
    AddMovementInput(Yaw.RotateVector(FVector::RightVector), Input.X);
}

void AActorPawn::Look(const FVector2D& Input)
{
    // Mouse XY is already a per-frame delta; do not multiply by DeltaSeconds.
    AddControllerYawInput(Input.X * LookSensitivity);
    AddControllerPitchInput(Input.Y * LookSensitivity);
}

void AActorPawn::Jump()
{
    bJumpRequested = true;
}

bool AActorPawn::InitializeCharacterMesh(USubMeshDataAsset* InDataAsset)
{
    if (!IsValid(InDataAsset) || !SubMeshControlComponent)
    {
        return false;
    }

    SubMeshControlComponent->RootTransform.SetLocation(FVector(0.0f, 0.0f, -CapsuleHeight));
    if (!SubMeshControlComponent->InitializeFromDataAsset(InDataAsset, Collision))
    {
        return false;
    }
    MeshAssetData = InDataAsset;
    return true;
}

void AActorPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (SubMeshControlComponent) SubMeshControlComponent->ClearGeneratedMeshes();
    MeshAssetData = nullptr;
    Super::EndPlay(EndPlayReason);
}

bool AActorPawn::FindGround(FHitResult& Hit) const
{
    const auto world = GetWorld();
    if (!world) return false;
    
    FCollisionQueryParams Params(SCENE_QUERY_STAT(ActorPawnGround), false, this);
    const FVector start = Collision->GetComponentLocation();
    const FVector end = start - FVector(0, 0, RaycastDistance);
    
    // スフィアトレースで地面を取得
    const bool bHit = world->SweepSingleByChannel(
        Hit,
        start,
        end,
        FQuat::Identity,
        Collision->GetCollisionObjectType(),
        FCollisionShape::MakeSphere(Collision->GetScaledCapsuleRadius()), Params,
        FCollisionResponseParams(Collision->GetCollisionResponseToChannels()));
    
    return bHit && !Hit.bStartPenetrating && Hit.ImpactNormal.Z >= 0.7f;
}

void AActorPawn::MoveWithCollision(const FVector& Delta)
{
    FHitResult Hit;
    AddActorWorldOffset(Delta, true, &Hit);
    if (Hit.IsValidBlockingHit())
    {
        // Continue the unused horizontal displacement along walls.
        FVector Slide = FVector::VectorPlaneProject(Delta * (1.0f - Hit.Time), Hit.Normal);
        Slide.Z = 0.0f;
        AddActorWorldOffset(Slide, true);
    }
}

void AActorPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    // 現在のフレームの移動入力ベクトル
    const FVector Input = ConsumeMovementInputVector().GetClampedToMaxSize(1.0f);
    bool bWantsJump = bJumpRequested;
    bJumpRequested = false;

    // Substeps reduce tunnelling and gravity integration error on slow frames.
    float Remaining = DeltaSeconds;
    while (Remaining > UE_SMALL_NUMBER)
    {
        const float Step = FMath::Min(Remaining, 1.0f / 120.0f);
        Remaining -= Step;

        FHitResult GroundHit;
        // 地面をチェック
        const bool bGrounded = VerticalSpeed <= 0.0f && FindGround(GroundHit);
        if (bGrounded)
        {
            VerticalSpeed = 0.0f;
            if (bWantsJump) VerticalSpeed = JumpSpeed;
        }
        bWantsJump = false;

        // 移動
        MoveWithCollision(FVector(Input.X, Input.Y, 0.0f) * MoveSpeed * Step);

        VerticalSpeed += GetWorld()->GetGravityZ() * GravityScale * Step;
        FHitResult VerticalHit;
        AddActorWorldOffset(FVector(0, 0, VerticalSpeed * Step), true, &VerticalHit);
        if (VerticalHit.IsValidBlockingHit())
        {
            // Includes landing and hitting a ceiling.
            VerticalSpeed = 0.0f;
        }
    }
}
