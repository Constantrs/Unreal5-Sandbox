// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SubMeshControlComponent.generated.h"

class USceneComponent;
class USkeletalMeshComponent;
class UActorDataAsset;

UCLASS(ClassGroup = (Rendering), meta = (BlueprintSpawnableComponent))
class UNREAL5SANDBOX_API USubMeshControlComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	USubMeshControlComponent();

	/** Runtime only. Null parent uses the owning actor's existing root component. */
	UFUNCTION(BlueprintCallable, Category = "Modular Mesh", meta = (UnsafeDuringActorConstruction = "true"))
	bool InitializeFromDataAsset(UActorDataAsset* InDataAsset, USceneComponent* AttachParent = nullptr);
	
	UFUNCTION(BlueprintCallable, Category = "Modular Mesh")
	void ClearGeneratedMeshes();

	UFUNCTION(BlueprintPure, Category = "Modular Mesh")
	USkeletalMeshComponent* GetRootMesh() const;

	UFUNCTION(BlueprintPure, Category = "Modular Mesh")
	TArray<USkeletalMeshComponent*> GetGeneratedMeshes() const;

	UFUNCTION(BlueprintPure, Category = "Modular Mesh")
	bool IsInitialized() const { return bInitialized; }
	
	// Relative to the attach parent. Set Z=-96 for the current ActorPawn capsule.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modular Mesh")
	FTransform RootTransform = FTransform::Identity;
	
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

private:
	UPROPERTY(Transient, VisibleInstanceOnly, Category = "Modular Mesh")
	bool bInitialized = false;
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<USkeletalMeshComponent>> GeneratedMeshes;
};
