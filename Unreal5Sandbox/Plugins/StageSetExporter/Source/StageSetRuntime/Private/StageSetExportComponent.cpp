#include "StageSetExportComponent.h"

UStageSetExportComponent::UStageSetExportComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bIsEditorOnly = true;
}
