#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "StageSetTableFactory.generated.h"

/** Adds a Stage Set Table entry to the Content Browser's asset creation menu. */
UCLASS()
class STAGESETEDITOR_API UStageSetTableFactory : public UFactory
{
    GENERATED_BODY()

public:
    UStageSetTableFactory();
    virtual uint32 GetMenuCategories() const override;
    virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name,
        EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};
