// Fill out your copyright notice in the Description page of Project Settings.


#include "EditorTextActor.h"

#if WITH_EDITORONLY_DATA
#include "Components/TextRenderComponent.h"
#endif

AEditorTextActor::AEditorTextActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Keep the Actor and its root available for future gameplay checkpoint logic.
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));

#if WITH_EDITORONLY_DATA
	EditorText = CreateEditorOnlyDefaultSubobject<UTextRenderComponent>(TEXT("EditorText"));
	if (EditorText)
	{
		EditorText->SetupAttachment(GetRootComponent());
		EditorText->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
		EditorText->SetHorizontalAlignment(EHTA_Center);
		EditorText->SetVerticalAlignment(EVRTA_TextCenter);
		EditorText->SetTextRenderColor(FColor::Yellow);
		EditorText->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		EditorText->SetGenerateOverlapEvents(false);
		EditorText->SetHiddenInGame(true);
		EditorText->SetText(EditorLabel);
		EditorText->SetWorldSize(EditorTextSize);
	}
#endif
}

void AEditorTextActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITORONLY_DATA
	FString info = FString::Printf(TEXT("Em %d\nIndex: %d"), Id, Index);
	EditorLabel = FText::FromString(info);
	if (EditorText)
	{
		EditorText->SetText(EditorLabel);
		EditorText->SetWorldSize(EditorTextSize);
	}
#endif
}