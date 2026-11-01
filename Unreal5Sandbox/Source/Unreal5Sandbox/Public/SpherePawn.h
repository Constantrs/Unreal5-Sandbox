#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SpherePawn.generated.h"

class UCameraComponent;
class UFloatingPawnMovement;
class USphereComponent;
class USpringArmComponent;
class UStaticMeshComponent;

/** A keyboard-controlled sphere that moves on the world XY plane. */
UCLASS()
class UNREAL5SANDBOX_API ASpherePawn : public APawn
{
	GENERATED_BODY()

public:
	ASpherePawn();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual UPawnMovementComponent* GetMovementComponent() const override;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sphere")
	TObjectPtr<USphereComponent> SphereCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sphere")
	TObjectPtr<UStaticMeshComponent> SphereMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UFloatingPawnMovement> Movement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;
private:
	void MoveForward(float Value);
	void MoveBackward(float Value);
	void MoveLeft(float Value);
	void MoveRight(float Value);
};
