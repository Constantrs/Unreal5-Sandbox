// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SideScrollerObjectTypes.h"

namespace SideScrollerCustomOptions
{
	enum class EParseResult : uint8 { NotProvided, Success, Invalid };

	/** SCObjects=1:1,2:101,3:201. Empty arrays produce SCObjects=. */
	SIDESCROLLERRUNTIME_API bool BuildCustomOptions(
		const TArray<FSideScrollerObjectInfo>& Objects, FString& OutOptions, FString& OutError);

	/** Consumes InitGame's Options string. On failure, OutObjects is empty. */
	SIDESCROLLERRUNTIME_API EParseResult ParseCustomOptions(
		const FString& Options, TArray<FSideScrollerObjectInfo>& OutObjects, FString& OutError);
}