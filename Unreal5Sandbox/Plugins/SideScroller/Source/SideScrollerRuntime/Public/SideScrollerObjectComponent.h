#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SideScrollerObjectTypes.h"
#include "SideScrollerObjectComponent.generated.h"

class ASideScrollerGameState;
class USkeletalMeshComponent;
class USkeletalMesh;

UCLASS(ClassGroup = (SideScroller), meta = (BlueprintSpawnableComponent))
class SIDESCROLLERRUNTIME_API USideScrollerObjectComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USideScrollerObjectComponent();

    UFUNCTION(BlueprintPure, Category = "SideScroller|Object")
    ESideScrollerCategory GetCategory() const { return Category; }

    UFUNCTION(BlueprintCallable, Category = "SideScroller|Object")
    void SetCategory(ESideScrollerCategory InCategory);

    UFUNCTION(BlueprintPure, Category = "SideScroller|Object")
    FSideScrollerObjectParam GetObjectParam() const { return ObjectParam; }
    
    UFUNCTION(BlueprintCallable, Category = "SideScroller|Object")
    void SetObjectParam(const FSideScrollerObjectParam& InParam) { ObjectParam = InParam; }
    
    UFUNCTION(BlueprintPure, Category = "SideScroller|Object")
    ESideScrollerHorizontalDirection GetHorizontalDirection() const { return HorizontalDirection; }

    UFUNCTION(BlueprintCallable, Category = "SideScroller|Object")
    void SetHorizontalDirection(ESideScrollerHorizontalDirection InDirection) { HorizontalDirection = InDirection; }

    UFUNCTION(BlueprintPure, Category = "SideScroller|Object")
    float GetHorizontalDirectionValue() const;

    /** Allows explicit retry if a custom flow creates GameState after this component. */
    UFUNCTION(BlueprintCallable, Category = "SideScroller|Object")
    bool RegisterWithGameState();

    UFUNCTION(BlueprintCallable, Category = "SideScroller|Object")
    void SetRootSkeletalMeshComponent(USkeletalMeshComponent* Component);

    UFUNCTION(BlueprintPure, Category = "SideScroller|Object")
    USkeletalMeshComponent* GetRootSkeletalMeshComponent() const;
    
    void SetRootMeshAsset(USkeletalMesh* Mesh);
    
protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SideScroller|Object")
    ESideScrollerCategory Category = ESideScrollerCategory::Undefined;

    UPROPERTY(Transient, VisibleInstanceOnly, BlueprintReadOnly, Category = "SideScroller|Object")
    FSideScrollerObjectParam ObjectParam;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SideScroller|Object")
    ESideScrollerHorizontalDirection HorizontalDirection = ESideScrollerHorizontalDirection::Right;
    
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UPROPERTY(Transient)
    TWeakObjectPtr<ASideScrollerGameState> RegisteredGameState;

    UPROPERTY(Transient)
    TObjectPtr<USkeletalMeshComponent> RootSkeletalMeshComponent;

    UPROPERTY(Transient)
    TObjectPtr<USkeletalMesh> InitialMesh;
};
