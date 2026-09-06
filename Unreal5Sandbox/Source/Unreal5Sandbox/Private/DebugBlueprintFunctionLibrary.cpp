// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugBlueprintFunctionLibrary.h"

#if WITH_EDITOR

void UDebugBlueprintFunctionLibrary::PrintScreenMessage(const FColor& Color, const FString& Message,
	float TimeToDisplay)
{
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, TimeToDisplay, Color, Message);		
	}
}

#endif