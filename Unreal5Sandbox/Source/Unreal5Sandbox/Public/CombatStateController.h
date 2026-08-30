// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatStateController.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	State_Idle = 1				UMETA(DisplayName = "Idle"),
	State_Run = 2				UMETA(DisplayName = "Run"),
	State_Land = 3				UMETA(DisplayName = "Land"),
	State_Jump = 4				UMETA(DisplayName = "Jump"),
	State_Fall = 5				UMETA(DisplayName = "Fall"),
	StateMaxIndex_OnGround = 3	UMETA(Hidden),
	StateMaxIndex_OnAir = 5		UMETA(Hidden),
	State_Undefined	= 0			UMETA(Hidden),
};

UCLASS( ClassGroup=(CombatSandbox), meta=(BlueprintSpawnableComponent) )
class UNREAL5SANDBOX_API UCombatStateController : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatStateController();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	ECombatState GetCombatState() const
	{
		return CurrentState;
	}

	void SetCombatState(ECombatState NewState)
	{
		CurrentState = NewState;
	}
	
protected:
	virtual void BeginPlay() override;

	virtual void SetDefaultState();
private:
	ECombatState CurrentState;
};
