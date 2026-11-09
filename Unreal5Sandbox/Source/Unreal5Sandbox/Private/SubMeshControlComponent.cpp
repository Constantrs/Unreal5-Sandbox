#include "SubMeshControlComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "ActorDataAsset.h"

DEFINE_LOG_CATEGORY_STATIC(LogDynamicSkeletalMesh, Log, All);

USubMeshControlComponent::USubMeshControlComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool USubMeshControlComponent::InitializeFromDataAsset(UActorDataAsset* InDataAsset, USceneComponent* AttachParent)
{
    AActor* Owner = GetOwner();
    if (!IsValid(Owner) || !GetWorld() || !GetWorld()->IsGameWorld()) return false;

    if (!AttachParent) AttachParent = Owner->GetRootComponent();
    if (!IsValid(AttachParent) || AttachParent->GetOwner() != Owner || !AttachParent->IsRegistered())
    {
        UE_LOG(LogDynamicSkeletalMesh, Warning, TEXT("A registered attach parent on the owning actor is required."));
        return false;
    }

    // A rebuild must not attach to a component that it is about to destroy.
    for (const auto& ExistingMesh : GeneratedMeshes)
    {
        if (IsValid(ExistingMesh.Get()) &&
            (AttachParent == ExistingMesh.Get() || AttachParent->IsAttachedTo(ExistingMesh.Get())))
        {
            UE_LOG(LogDynamicSkeletalMesh, Warning, TEXT("Attach parent cannot belong to the generated mesh hierarchy."));
            return false;
        }
    }


    if (!IsValid(InDataAsset) || InDataAsset->SkeletonAssets.IsEmpty())
    {
        UE_LOG(LogDynamicSkeletalMesh, Warning, TEXT("DataAsset must contain at least one entry."));
        return false;
    }

    const int32 subMeshNum =  InDataAsset->SkeletonAssets.Num();
    if (!InDataAsset->AnimClass.IsNull() && !InDataAsset->AnimClass.Get())
    {
        UE_LOG(LogDynamicSkeletalMesh, Warning, TEXT("Animation class must be loaded before initialization."));
        return false;
    }
    
    for (int32 Index = 0; Index < subMeshNum; ++Index)
    {
        if (!IsValid(InDataAsset->SkeletonAssets[Index].SkeletonAsset.Get()))
        {
            UE_LOG(LogDynamicSkeletalMesh, Warning, TEXT("Entry %d has no Skeletal Mesh asset."), Index);
            return false;
        }
    }

    // Validate first so invalid input leaves the current mesh hierarchy intact.
    ClearGeneratedMeshes();
    GeneratedMeshes.Reserve(subMeshNum);

    for (int32 Index = 0; Index < subMeshNum; ++Index)
    {
        const FActorSkeletonAssetEntry& data = InDataAsset->SkeletonAssets[Index];
        // NewObject chooses a unique UObject name, even when asset names repeat.
        USkeletalMeshComponent* Part = NewObject<USkeletalMeshComponent>(Owner, data.AssetName, RF_Transient);
        Owner->AddInstanceComponent(Part);
        GeneratedMeshes.Add(Part);
        if (!data.AssetName.IsNone()) Part->ComponentTags.Add(data.AssetName);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Part->SetGenerateOverlapEvents(false);
        Part->SetSkeletalMeshAsset(data.SkeletonAsset.Get());

        // Root
        if (Index == 0)
        {
            Part->SetupAttachment(AttachParent);
            Part->SetRelativeTransform(RootTransform);
            // The leader must refresh bones even when only followers are visible.
            Part->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
            if (UClass* AnimClass = InDataAsset->AnimClass.Get())
            {
                Part->SetAnimInstanceClass(AnimClass);
            }
        }
        else
        {
            USkeletalMeshComponent* Leader = GetRootMesh();
            Part->SetupAttachment(Leader);
            Part->SetRelativeTransform(FTransform::Identity);
            Part->SetLeaderPoseComponent(Leader, true, false);
        }

        Part->RegisterComponent();
        if (!Part->IsRegistered())
        {
            UE_LOG(LogDynamicSkeletalMesh, Error, TEXT("Failed to register mesh entry %d."), Index);
            ClearGeneratedMeshes();
            return false;
        }
    }
    bInitialized = true;
    return true;
}

void USubMeshControlComponent::ClearGeneratedMeshes()
{
    bInitialized = false;
    // Destroy followers before the leader.
    for (int32 Index = GeneratedMeshes.Num() - 1; Index >= 0; --Index)
    {
        USkeletalMeshComponent* Part = GeneratedMeshes[Index].Get();
        if (!IsValid(Part)) continue;
        Part->SetLeaderPoseComponent(nullptr, true, false);
        if (AActor* Owner = GetOwner()) Owner->RemoveInstanceComponent(Part);
        Part->DestroyComponent();
    }
    GeneratedMeshes.Empty();
}

USkeletalMeshComponent* USubMeshControlComponent::GetRootMesh() const
{
    return GeneratedMeshes.IsEmpty() ? nullptr : GeneratedMeshes[0].Get();
}

TArray<USkeletalMeshComponent*> USubMeshControlComponent::GetGeneratedMeshes() const
{
    TArray<USkeletalMeshComponent*> Result;
    Result.Reserve(GeneratedMeshes.Num());
    for (const auto& Part : GeneratedMeshes) Result.Add(Part.Get());
    return Result;
}

void USubMeshControlComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (bInitialized)
    {
        ClearGeneratedMeshes();
    }
    Super::EndPlay(EndPlayReason);
}

void USubMeshControlComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
    ClearGeneratedMeshes();
    Super::OnComponentDestroyed(bDestroyingHierarchy);
}
