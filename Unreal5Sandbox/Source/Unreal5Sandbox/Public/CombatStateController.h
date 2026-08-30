// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatStateController.generated.h"

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	State_Idle = 0		UMETA(DisplayName = "Idle"),
	State_Run = 1		UMETA(DisplayName = "Run"),
	Undefined			UMETA(Hidden),
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
