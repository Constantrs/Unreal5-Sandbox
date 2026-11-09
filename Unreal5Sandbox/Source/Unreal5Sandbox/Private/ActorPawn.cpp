#include "ActorPawn.h"

#include "Camera/CameraComponent.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "SubMeshControlComponent.h"
#include "AnimMontageController.h"
#include "ActorDataAsset.h"

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

    // モンタージュ管理コンポーネント
    AnimationMontageController = CreateDefaultSubobject<UAnimMontageController>(TEXT("AnimMontageController"));
    
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

    FRotator ViewRotation = Camera->GetComponentRotation();
    FVector Forward = ViewRotation.Vector();
    Forward.Z = 0.0f;
    if (!Forward.Normalize(KINDA_SMALL_NUMBER))
    {
        // A vertical camera has no horizontal forward projection. Use its yaw
        // to provide a stable movement basis instead of normalizing near-zero noise.
        Forward = FRotator(0.0f, ViewRotation.Yaw, 0.0f).Vector();
    }
    const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward);

    // Face camera-forward for every movement direction, including strafing/backward.
    // Yaw rotates around world Z in Unreal; preserve the current pitch and roll.
    FRotator FacingRotation = GetActorRotation();
    const float TargetYaw = Forward.Rotation().Yaw;
    if (!FMath::IsNearlyZero(FMath::FindDeltaAngleDegrees(FacingRotation.Yaw, TargetYaw), 0.1f))
    {
        FacingRotation.Yaw = TargetYaw;
        SetActorRotation(FacingRotation);
    }
    
    AddMovementInput(Forward, Input.Y);
    AddMovementInput(Right, Input.X);
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

bool AActorPawn::InitializeActorMesh(UActorDataAsset* InDataAsset)
{
    if (!IsValid(InDataAsset) || !SubMeshControlComponent)
    {
        return false;
    }

    SubMeshControlComponent->RootTransform.SetLocation(FVector(0.0f, 0.0f, -CapsuleHeight));
    SubMeshControlComponent->RootTransform.SetRotation(FRotator(0.0f, -90.0f, 0.0f).Quaternion());
    if (!SubMeshControlComponent->InitializeFromDataAsset(InDataAsset, Collision))
    {
        return false;
    }
    MeshAssetData = InDataAsset;
    AnimationMontageController->SetTargetMesh( SubMeshControlComponent->GetRootMesh());
    return true;
}

void AActorPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (AnimationMontageController) AnimationMontageController->SetTargetMesh(nullptr);
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
    const FVector end = start - FVector(0, 0, Collision->GetScaledCapsuleHalfHeight());
    const FQuat rotation = Collision->GetComponentQuat();
    
    // スフィアトレースで地面を取得
    const bool bHit = world->SweepSingleByChannel(
        Hit,
        start,
        end,
        rotation,
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
