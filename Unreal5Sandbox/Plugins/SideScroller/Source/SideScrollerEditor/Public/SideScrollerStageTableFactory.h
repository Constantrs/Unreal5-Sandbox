#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "SideScrollerStageTableFactory.generated.h"

UCLASS()
class SIDESCROLLEREDITOR_API USideScrollerStageTableFactory : public UFactory
{
    GENERATED_BODY()

public:
    USideScrollerStageTableFactory();
    virtual uint32 GetMenuCategories() const override;
    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name,
        EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
