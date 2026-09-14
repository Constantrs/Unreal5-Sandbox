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
	State_Dash = 3				UMETA(DisplayName = "Dash"),
	State_Land = 4				UMETA(DisplayName = "Land"),
	State_Jump = 5				UMETA(DisplayName = "Jump"),
	State_AirDash = 6			UMETA(DisplayName = "AirDash"),
	State_Fall = 7				UMETA(DisplayName = "Fall"),
	StateMaxIndex_OnGround = 4	UMETA(Hidden),
	StateMaxIndex_OnAir = 7		UMETA(Hidden),
	State_Undefined	= 0			UMETA(Hidden),
};

UCLASS( ClassGroup=(CombatSandbox), meta=(BlueprintSpawnableComponent) )
class UNREAL5SANDBOX_API UCombatStateController : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatStateController();

#ifdef WITH_EDITOR
	void PrintCurrentState() const;
#endif
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	ECombatState GetCombatState() const
	{
		return CurrentState;
	}

	void SetCombatState(ECombatState NewState, bool PrintState = true);
	
protected:
	virtual void BeginPlay() override;

	virtual void SetDefaultState();

private:
	ECombatState CurrentState;
};
