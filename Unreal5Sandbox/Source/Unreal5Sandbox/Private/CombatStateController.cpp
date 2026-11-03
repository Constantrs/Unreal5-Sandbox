#include "CombatStateController.h"

#ifdef WITH_EDITOR
#include "DebugBlueprintFunctionLibrary.h"
#endif

UCombatStateController::UCombatStateController()
{
	PrimaryComponentTick.bCanEverTick = true;
}

#ifdef WITH_EDITOR
void UCombatStateController::PrintCurrentState() const
{
	FString CurrentStateString = UEnum::GetValueAsString(CurrentState);
	UDebugBlueprintFunctionLibrary::PrintScreenMessage(FColor::Yellow, CurrentStateString);
}
#endif

void UCombatStateController::SetCombatState(ECombatState NewState, bool PrintState)
{
	CurrentState = NewState;
	if (PrintState)
	{
#ifdef WITH_EDITOR
		PrintCurrentState();
#endif
	}
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

