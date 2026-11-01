#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ActorPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class AActorPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;

	UPROPERTY(EditAnywhere, Category = "Controller|Input")
	bool InverseYAxis = true;
	
	UPROPERTY(EditDefaultsOnly, Category = "Controller|Input")
	TObjectPtr<UInputMappingContext> PlayerMappingContext;
	UPROPERTY(EditDefaultsOnly, Category = "Controller|Input")
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(EditDefaultsOnly, Category = "Controller|Input")
	TObjectPtr<UInputAction> LookAction;
	UPROPERTY(EditDefaultsOnly, Category = "Controller|Input")
	TObjectPtr<UInputAction> JumpAction;

private:
	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleJump(const FInputActionValue& Value);
};
