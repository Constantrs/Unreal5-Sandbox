#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ActorPawn.generated.h"

class UCapsuleComponent;
class USkeletalMeshComponent;
class USpringArmComponent;
class UCameraComponent;

UCLASS()
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
protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UCapsuleComponent> Collision;
	UPROPERTY(VisibleAnywhere, Category = "ActorPawn|Components")
	TObjectPtr<USkeletalMeshComponent> RootMesh;
	UPROPERTY(VisibleAnywhere, Category = "ActorPawn|Components")
	TObjectPtr<USpringArmComponent> SpringArm;
	UPROPERTY(VisibleAnywhere, Category = "ActorPawn|Components")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditAnywhere, Category = "ActorPawn|Movement", meta = (ClampMin = "0"))
	float MoveSpeed = 600.0f;
	UPROPERTY(EditAnywhere, Category = "ActorPawn|Movement", meta = (ClampMin = "0"))
	float JumpSpeed = 600.0f;
	UPROPERTY(EditAnywhere, Category = "ActorPawn|Movement", meta = (ClampMin = "0"))
	float GravityScale = 1.0f;
	UPROPERTY(EditAnywhere, Category = "ActorPawn|Camera")
	float LookSensitivity = 1.0f;
	
private:
	bool FindGround(FHitResult& Hit) const;
	void MoveWithCollision(const FVector& Delta);
	
	float VerticalSpeed = 0.0f;
	bool bJumpRequested = false;
};
