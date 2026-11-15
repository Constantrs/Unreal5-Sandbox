#include "SideScrollerStageTableFactory.h"
#include "AssetTypeCategories.h"
#include "SideScrollerStageTable.h"

USideScrollerStageTableFactory::USideScrollerStageTableFactory()
{
    SupportedClass = USideScrollerStageTable::StaticClass();
    bCreateNew = true;
    bEditAfterNew = true;
}

uint32 USideScrollerStageTableFactory::GetMenuCategories() const
{
    return EAssetTypeCategories::Misc;
}

UObject* USideScrollerStageTableFactory::FactoryCreateNew(UClass* Class, UObject* InParent,
    FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
    return NewObject<USideScrollerStageTable>(InParent, Class, Name, Flags | RF_Transactional);
}
