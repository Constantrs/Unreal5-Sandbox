#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "SideScrollerObjectTypes.h"
#include "SideScrollerGameState.generated.h"

USTRUCT()
struct FSideScrollerObjectList
{
    GENERATED_BODY()

    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<AActor>> Objects;
};

/** Local world registry. This map is not replicated. */
UCLASS(Blueprintable, ClassGroup = (SideScroller), meta = (PrioritizeCategories = "SideScroller"))
class SIDESCROLLERRUNTIME_API ASideScrollerGameState : public AGameState
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "SideScroller|Objects")
    bool RegisterObject(AActor* Object);

    UFUNCTION(BlueprintCallable, Category = "SideScroller|Objects")
    void UnregisterObject(AActor* Object);

    /** Returns a snapshot; does not expose the registry's mutable storage. */
    UFUNCTION(BlueprintCallable, Category = "SideScroller|Objects")
    TArray<AActor*> GetObjectsByCategory(ESideScrollerCategory Category);

    UFUNCTION(BlueprintCallable, Category = "SideScroller|Objects")
    void PruneInvalidObjects();

private:
    UPROPERTY(Transient)
    TMap<ESideScrollerCategory, FSideScrollerObjectList> ObjectsByCategory;
};
