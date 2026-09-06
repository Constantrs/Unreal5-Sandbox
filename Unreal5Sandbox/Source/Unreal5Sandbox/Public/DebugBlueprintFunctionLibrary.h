// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DebugBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class UNREAL5SANDBOX_API UDebugBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

#if WITH_EDITOR
	static void PrintScreenMessage(const FColor& Color, const FString& Message, float TimeToDisplay = 12.0f);
#endif
	
};
