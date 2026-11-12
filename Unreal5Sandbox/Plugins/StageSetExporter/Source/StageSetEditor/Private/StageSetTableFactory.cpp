#include "StageSetTableFactory.h"

#include "AssetTypeCategories.h"
#include "StageSetTable.h"

UStageSetTableFactory::UStageSetTableFactory()
{
    SupportedClass = UStageSetTable::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

uint32 UStageSetTableFactory::GetMenuCategories() const
{
    return EAssetTypeCategories::Misc;
}

UObject* UStageSetTableFactory::FactoryCreateNew(UClass* Class, UObject* InParent,
    FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    return NewObject<UStageSetTable>(InParent, Class, Name, Flags | RF_Transactional);
}
