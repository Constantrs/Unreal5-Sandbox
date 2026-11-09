// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EditorTextActor.generated.h"

class UTextRenderComponent;

UCLASS(PrioritizeCategories = ("EditorText"))
class UNREAL5SANDBOX_API AEditorTextActor : public AActor
{
	GENERATED_BODY()
	
public:
	AEditorTextActor();

	virtual void OnConstruction(const FTransform& Transform) override;

private:
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditInstanceOnly, Category = "EditorText|Font", meta = (ClampMin = "1.0"))
	float EditorTextSize = 40.0f;
	
	/* Editor-only Description String */
	UPROPERTY(VisibleAnywhere, Category = "EditorText|Font", meta = (MultiLine = true))
	FText EditorLabel;

	UPROPERTY(VisibleAnywhere, Category = "EditorText|Font")
	TObjectPtr<UTextRenderComponent> EditorText;

	UPROPERTY(EditInstanceOnly, Category = "EditorText|Info", meta = (ClampMin = "1"))
	int32 Id = 0;
	
	UPROPERTY(EditInstanceOnly, Category = "EditorText|Info", meta = (ClampMin = "0"))
	int32 Index = 0;
#endif

};
