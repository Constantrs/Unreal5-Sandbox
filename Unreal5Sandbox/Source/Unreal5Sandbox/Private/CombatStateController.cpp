// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatStateController.h"

UCombatStateController::UCombatStateController()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UCombatStateController::BeginPlay()
{
	Super::BeginPlay();
	
	SetDefaultState();
}

void UCombatStateController::SetDefaultState()
{
	SetCombatState(ECombatState::State_Idle);
}


void UCombatStateController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

