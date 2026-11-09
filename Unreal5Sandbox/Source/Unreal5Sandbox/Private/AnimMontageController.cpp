// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimMontageController.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogAnimationMontageComponent, Log, All);

UAnimMontageController::UAnimMontageController()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAnimMontageController::SetTargetMesh(USkeletalMeshComponent* InMesh)
{
	if (TargetMesh.Get() == InMesh) return;
	StopMontage(0.0f);
	TargetMesh = InMesh;
}

bool UAnimMontageController::PlayMontage(UAnimMontage* Montage, float BlendRate)
{
	if (!IsValid(Montage) || !FMath::IsFinite(CurrentPlayRate) || CurrentPlayRate <= 0.0f)
	{
		UE_LOG(LogAnimationMontageComponent, Warning, TEXT("Invalid Montage, blend-in duration or playback rate."));
		return false;
	}

	auto blendRate = FMath::Clamp(BlendRate, 0.0f, 1.0f);
	
	USkeletalMeshComponent* Mesh = TargetMesh.Get();
	UAnimInstance* AnimInstance = Mesh ? Mesh->GetAnimInstance() : nullptr;
	if (!AnimInstance || Mesh->GetAnimationMode() != EAnimationMode::AnimationBlueprint)
	{
		UE_LOG(LogAnimationMontageComponent, Warning, TEXT("Assign a target mesh with an active Animation Blueprint before playing."));
		return false;
	}

	FAlphaBlendArgs BlendIn(blendRate);
	const float Result = AnimInstance->Montage_PlayWithBlendIn(
		Montage, BlendIn, CurrentPlayRate,
		EMontagePlayReturnType::MontageLength, 0.0f, false);
	if (Result <= 0.0f) return false;

	PlayingAnimInstance = AnimInstance;
	CurrentMontage = Montage;
	return true;
}

bool UAnimMontageController::SetCurrentPlayRate(float NewRate)
{
	if (!FMath::IsFinite(NewRate) || NewRate <= 0.0f) return false;

	CurrentPlayRate = NewRate;
	if (UAnimInstance* AnimInstance = PlayingAnimInstance.Get())
	{
		if (CurrentMontage && AnimInstance->Montage_IsActive(CurrentMontage))
			AnimInstance->Montage_SetPlayRate(CurrentMontage, CurrentPlayRate);
	}
	return true;
}

void UAnimMontageController::StopMontage(float BlendOutTime)
{
	if (!FMath::IsFinite(BlendOutTime) || BlendOutTime < 0.0f) return;

	if (UAnimInstance* AnimInstance = PlayingAnimInstance.Get())
	{
		if (CurrentMontage) AnimInstance->Montage_Stop(BlendOutTime, CurrentMontage);
	}
	CurrentMontage = nullptr;
	PlayingAnimInstance.Reset();
	// Preserve CurrentPlayRate so the next Montage inherits the same setting.
}

void UAnimMontageController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopMontage(0.0f);
	TargetMesh.Reset();
	Super::EndPlay(EndPlayReason);
}
