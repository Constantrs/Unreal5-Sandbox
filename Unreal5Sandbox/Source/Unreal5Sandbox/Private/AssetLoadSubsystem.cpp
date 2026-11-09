#include "AssetLoadSubsystem.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Engine/AssetManager.h"
#include "Engine/SkeletalMesh.h"
#include "ActorDataAsset.h"

DEFINE_LOG_CATEGORY_STATIC(LogCharacterAssets, Log, All);

UActorDataAsset* UAssetLoadSubsystem::AcquireActorAssetSync(FPrimaryAssetId AssetId)
{
	check(IsInGameThread());
    
    if (!AssetId.IsValid() || AssetId.PrimaryAssetType != FPrimaryAssetType(TEXT("ActorDataAsset")))
    {
        UE_LOG(LogCharacterAssets, Warning, TEXT("Invalid character PrimaryAssetId: %s"), *AssetId.ToString());
        return nullptr;
    }
    if (FLoadedActorAsset* Existing = LoadedAssets.Find(AssetId))
    {
        ++Existing->Users;
        return Existing->ActorAssets.Get();
    }

    UAssetManager& manager = UAssetManager::Get();
    const FSoftObjectPath Path = manager.GetPrimaryAssetPath(AssetId);
    if (!Path.IsValid())
    {
        UE_LOG(LogCharacterAssets, Error, TEXT("Primary asset not registered: %s. Check Asset Manager scan settings."), *AssetId.ToString());
        return nullptr;
    }

    FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
    UActorDataAsset* Data = Cast<UActorDataAsset>(Streamable.LoadSynchronous(Path));
    if (!Data || Data->GetPrimaryAssetId() != AssetId || Data->SkeletonAssets.IsEmpty())
    {
        UE_LOG(LogCharacterAssets, Error, TEXT("Failed to load a valid, non-empty character definition: %s"), *AssetId.ToString());
        return nullptr;
    }

    // Keep the definition alive while its secondary assets are synchronously loaded.
    LoadedAssets.Add(AssetId).ActorAssets = Data;
    for (const FActorSkeletonAssetEntry& Entry : Data->SkeletonAssets)
    {
        USkeletalMesh* Mesh = Streamable.LoadSynchronous(Entry.SkeletonAsset);
        if (!Mesh)
        {
            UE_LOG(LogCharacterAssets, Error, TEXT("%s: failed to load mesh '%s' (%s)."),
                *AssetId.ToString(), *Entry.AssetName.ToString(), *Entry.SkeletonAsset.ToString());
            LoadedAssets.Remove(AssetId);
            return nullptr;
        }
        LoadedAssets.FindChecked(AssetId).Resources.AddUnique(Mesh);
    }
    if (!Data->AnimClass.IsNull())
    {
        const TSubclassOf<UAnimInstance> AnimClass = Streamable.LoadSynchronous(Data->AnimClass);
        if (!AnimClass)
        {
            UE_LOG(LogCharacterAssets, Error, TEXT("%s: failed to load animation class."), *AssetId.ToString());
            LoadedAssets.Remove(AssetId);
            return nullptr;
        }
        LoadedAssets.FindChecked(AssetId).Resources.AddUnique(AnimClass.Get());
    }
    LoadedAssets.FindChecked(AssetId).Users = 1;
    return Data;
}

void UAssetLoadSubsystem::ReleaseActorAsset(FPrimaryAssetId AssetId)
{
    check(IsInGameThread());
    if (FLoadedActorAsset* Entry = LoadedAssets.Find(AssetId))
    {
        if (--Entry->Users <= 0) LoadedAssets.Remove(AssetId);
    }
    // We use unmanaged synchronous streamable loads, not LoadPrimaryAsset state.
    // Removing our strong references permits GC after all components release theirs.
}

void UAssetLoadSubsystem::Deinitialize()
{
    LoadedAssets.Empty();
    Super::Deinitialize();
}
