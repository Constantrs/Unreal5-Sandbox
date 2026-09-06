// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "CombatCameraManager.generated.h"

/**
 * 
 */
UCLASS()
class UNREAL5SANDBOX_API ACombatCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	float CrouchBlendDuration = 0.25f;

	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;
private:
	float CrouchBlendTime = 0.0f;
};
