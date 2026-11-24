#include "SideScrollerStageConfigAsset.h"

#include "Engine/World.h"
#include "GenericPlatform/GenericPlatformHttp.h"
#include "Kismet/GameplayStatics.h"

FString USideScrollerStageConfigAsset::BuildStageOptions(FName Id)
{
    return TEXT("Stage_") + Id.ToString();
}

bool USideScrollerStageConfigAsset::ParseStageOptions(const FString& Options, FName& OutId, FString& OutError)
{
    OutId = NAME_None;
    OutError.Reset();
    if (!UGameplayStatics::HasOption(Options, TEXT("StageId"))) return true;
    const FString Text = UGameplayStatics::ParseOption(Options, TEXT("StageId"));
    if (Text.IsEmpty() || Text.Len() >= NAME_SIZE || FName(*Text).IsNone())
    {
        OutError = TEXT("StageId option is empty or invalid.");
        return false;
    }
    OutId = FName(*Text);
    return true;
}

bool USideScrollerStageConfigAsset::ValidateStageIds(FString& OutError) const
{
    OutError.Reset();
    TSet<FName> Ids;
    for (int32 Index = 0; Index < Stages.Num(); ++Index)
    {
        const FName Id = Stages[Index].StageId;
        if (Id.IsNone() || Ids.Contains(Id))
        {
            OutError = FString::Printf(TEXT("Stages[%d]: StageId is empty or duplicated (%s)."), Index, *Id.ToString());
            return false;
        }
        Ids.Add(Id);
    }
    return true;
}

bool USideScrollerStageConfigAsset::ResolveStage(FName RequestedId, const FString& MapPackageName, FName& OutId,
                                                 FSideScrollerStageConfig& OutConfig, FString& OutError) const
{
    OutId = NAME_None;
    OutConfig = FSideScrollerStageConfig();
    OutError.Reset();
    if (!ValidateStageIds(OutError)) return false;
    const FSideScrollerStageConfig* Config = nullptr;
    if (!RequestedId.IsNone())
    {
        Config = Stages.FindByPredicate([RequestedId](const FSideScrollerStageConfig& Stage) { return Stage.StageId == RequestedId; });
        OutId = RequestedId;
        if (!Config)
        {
            OutError = FString::Printf(TEXT("StageId %s is not in StageConfigs."), *RequestedId.ToString());
            return false;
        }
    }
    else
    {
        const FString Map = UWorld::RemovePIEPrefix(MapPackageName);
        for (const FSideScrollerStageConfig& Stage : Stages)
        {
            if (!Stage.Level.IsNull() && Stage.Level.ToSoftObjectPath().GetLongPackageName() == Map)
            {
                if (Config)
                {
                    OutError = TEXT("Multiple stages use this map; supply StageId explicitly.");
                    return false;
                }
                Config = &Stage;
                OutId = Stage.StageId;
            }
        }
        if (!Config)
        {
            OutError = TEXT("No StageId was supplied and no StageConfigs Level matches this map.");
            return false;
        }
    }
    if (Config->EnemyAssets.IsNull() || Config->EnemySpawnTable.IsNull())
    {
        OutError = FString::Printf(TEXT("StageId %s needs EnemyAssets and EnemySpawnTable soft references."), *OutId.ToString());
        return false;
    }
    OutConfig = *Config;
    return true;
}
