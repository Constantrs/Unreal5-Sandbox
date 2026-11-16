#include "SideScrollerGameState.h"

#include "SideScrollerObject.h"

bool ASideScrollerGameState::RegisterObject(AActor* Object)
{
    if (!IsValid(Object) || Object->IsActorBeingDestroyed() || Object->GetWorld() != GetWorld() ||
        !Object->GetClass()->ImplementsInterface(USideScrollerObject::StaticClass())) return false;

    const ESideScrollerCategory Category = ISideScrollerObject::Execute_GetCategory(Object);
    // Also removes any old category entry when re-registering an object.
    UnregisterObject(Object);
    ObjectsByCategory.FindOrAdd(Category).Objects.Add(Object);
    return true;
}

void ASideScrollerGameState::UnregisterObject(AActor* Object)
{
    for (auto It = ObjectsByCategory.CreateIterator(); It; ++It)
    {
        It.Value().Objects.RemoveAll([Object](const TWeakObjectPtr<AActor>& Entry)
        {
            return !Entry.IsValid() || Entry.Get() == Object;
        });
        if (It.Value().Objects.IsEmpty()) It.RemoveCurrent();
    }
}

void ASideScrollerGameState::PruneInvalidObjects()
{
    UnregisterObject(nullptr);
}

TArray<AActor*> ASideScrollerGameState::GetObjectsByCategory(ESideScrollerCategory Category)
{
    PruneInvalidObjects();
    TArray<AActor*> Result;
    if (const FSideScrollerObjectList* List = ObjectsByCategory.Find(Category))
    {
        for (const TWeakObjectPtr<AActor>& Entry : List->Objects)
        {
            if (AActor* Object = Entry.Get()) Result.Add(Object);
        }
    }
    return Result;
}
