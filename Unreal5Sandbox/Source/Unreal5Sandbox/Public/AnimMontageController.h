// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AnimMontageController.generated.h"

class UAnimInstance;
class UAnimMontage;
class USkeletalMeshComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREAL5SANDBOX_API UAnimMontageController : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAnimMontageController();

	UFUNCTION(BlueprintCallable, Category = "Animation|Montage")
	void SetTargetMesh(USkeletalMeshComponent* InMesh);

	/** BlendRate is blend-in duration in seconds, not playback speed. */
	UFUNCTION(BlueprintCallable, Category = "Animation|Montage", meta = (UnsafeDuringActorConstruction = "true"))
	bool PlayMontage(UAnimMontage* Montage, float BlendRate = 0.2f);

	/** Positive finite rates only. Also updates this component's current playback. */
	UFUNCTION(BlueprintCallable, Category = "Animation|Montage")
	bool SetCurrentPlayRate(float NewRate);

	UFUNCTION(BlueprintPure, Category = "Animation|Montage")
	float GetCurrentPlayRate() const { return CurrentPlayRate; }

	UFUNCTION(BlueprintCallable, Category = "Animation|Montage")
	void StopMontage(float BlendOutTime = 0.2f);

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation|Montage", meta = (AllowPrivateAccess = "true", ClampMin = "0.01"))
	float CurrentPlayRate = 1.0f;

	UPROPERTY(Transient)
	TWeakObjectPtr<USkeletalMeshComponent> TargetMesh;

	UPROPERTY(Transient)
	TWeakObjectPtr<UAnimInstance> PlayingAnimInstance;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> CurrentMontage;
};
