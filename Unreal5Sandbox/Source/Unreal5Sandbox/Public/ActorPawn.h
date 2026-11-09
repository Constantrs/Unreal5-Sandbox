#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ActorPawn.generated.h"

class UCapsuleComponent;
class USkeletalMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UActorDataAsset;
class USubMeshControlComponent;
class UAnimMontageController;

UCLASS(PrioritizeCategories = ("ActorPawn"))
class UNREAL5SANDBOX_API AActorPawn : public APawn
{
	GENERATED_BODY()

public:
	AActorPawn();
	virtual void Tick(float DeltaSeconds) override;

	// Input.X = right, Input.Y = forward.
	void Move(const FVector2D& Input);
	void Look(const FVector2D& Input);
	void Jump();
	
	UFUNCTION(BlueprintCallable, Category = "Modular Mesh")
	bool InitializeActorMesh(UActorDataAsset* InDataAsset);
protected:
	UPROPERTY(VisibleAnywhere, Category = "ActorPawn|Components")
	TObjectPtr<UCapsuleComponent> Collision;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActorPawn|Components")
	TObjectPtr<USubMeshControlComponent> SubMeshControlComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ActorPawn|Components")
	TObjectPtr<UAnimMontageController> AnimationMontageController;
	
	UPROPERTY(VisibleAnywhere, Category = "ActorPawn|Components")
	TObjectPtr<USpringArmComponent> SpringArm;
	UPROPERTY(VisibleAnywhere, Category = "ActorPawn|Components")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, Category = "ActorPawn|Collision", meta = (ClampMin = "0"))
	float CapsuleHeight = 90.0f;
	UPROPERTY(EditAnywhere, Category = "ActorPawn|Collision", meta = (ClampMin = "0"))
	float CapsuleRadius = 40.0f;
	UPROPERTY(EditAnywhere, Category = "ActorPawn|Collision", meta = (ClampMin = "0"))
	float RaycastDistance = 50.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ActorPawn|Reference")
	TObjectPtr<UActorDataAsset> MeshAssetData;
	
	UPROPERTY(EditAnywhere, Category = "ActorPawn|Movement", meta = (ClampMin = "0"))
	float MoveSpeed = 600.0f;
	UPROPERTY(EditAnywhere, Category = "ActorPawn|Movement", meta = (ClampMin = "0"))
	float JumpSpeed = 600.0f;
	UPROPERTY(EditAnywhere, Category = "ActorPawn|Movement", meta = (ClampMin = "0"))
	float GravityScale = 1.0f;
	UPROPERTY(EditAnywhere, Category = "ActorPawn|Camera")
	float LookSensitivity = 1.0f;
	
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	bool FindGround(FHitResult& Hit) const;
	void MoveWithCollision(const FVector& Delta);
	
	float VerticalSpeed = 0.0f;
	bool bJumpRequested = false;
};
