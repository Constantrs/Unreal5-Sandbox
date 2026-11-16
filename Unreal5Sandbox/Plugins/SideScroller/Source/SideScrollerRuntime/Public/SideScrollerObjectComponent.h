#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SideScrollerObjectTypes.h"
#include "SideScrollerObjectComponent.generated.h"

class ASideScrollerGameState;

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
    ESideScrollerHorizontalDirection GetHorizontalDirection() const { return HorizontalDirection; }

    UFUNCTION(BlueprintCallable, Category = "SideScroller|Object")
    void SetHorizontalDirection(ESideScrollerHorizontalDirection InDirection) { HorizontalDirection = InDirection; }

    UFUNCTION(BlueprintPure, Category = "SideScroller|Object")
    float GetHorizontalDirectionValue() const;

    /** Allows explicit retry if a custom flow creates GameState after this component. */
    UFUNCTION(BlueprintCallable, Category = "SideScroller|Object")
    bool RegisterWithGameState();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SideScroller|Object")
    ESideScrollerCategory Category = ESideScrollerCategory::Undefined;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SideScroller|Object")
    ESideScrollerHorizontalDirection HorizontalDirection = ESideScrollerHorizontalDirection::Right;
    
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UPROPERTY(Transient)
    TWeakObjectPtr<ASideScrollerGameState> RegisteredGameState;
};
