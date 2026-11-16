#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SideScrollerObjectTypes.h"
#include "SideScrollerObject.generated.h"

UINTERFACE(BlueprintType, Blueprintable)
class SIDESCROLLERRUNTIME_API USideScrollerObject : public UInterface
{
    GENERATED_BODY()
};

class SIDESCROLLERRUNTIME_API ISideScrollerObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "SideScroller|Object")
    ESideScrollerCategory GetCategory() const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "SideScroller|Object")
    ESideScrollerHorizontalDirection GetHorizontalDirection() const;

    UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "SideScroller|Object")
    float GetHorizontalDirectionValue() const;
};
