#include "StageSetExportWidget.h"

#include "DataTableEditorUtils.h"
#include "DetailsViewArgs.h"
#include "Editor.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "IDetailsView.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "ScopedTransaction.h"
#include "StageSetExportComponent.h"
#include "StageSetTable.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "Subsystems/EditorAssetSubsystem.h"
#include "UObject/Package.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

DEFINE_LOG_CATEGORY_STATIC(LogStageSetExporter, Log, All);

namespace StageSetExporter
{
    FStageSetRow MakeRow(const AActor& Actor, const UStageSetExportComponent& Source)
    {
        FStageSetRow Row;
        Row.Category = Source.Category;
        Row.Id = Source.Id;
        Row.AreaId = Source.AreaId;
        Row.Transform = Actor.GetActorTransform();
        Row.Param01 = Source.Param01;
        Row.Param02 = Source.Param02;
        Row.Param03 = Source.Param03;
        Row.Param04 = Source.Param04;
        return Row;
    }
}

TSharedRef<SWidget> UStageSetExportWidget::RebuildWidget()
{
    if (!Settings)
    {
        Settings = NewObject<UStageSetExportSettings>(this);
        Settings->ActorClass = AActor::StaticClass();
    }

    FDetailsViewArgs Args;
    Args.bAllowSearch = true;
    Args.bHideSelectionTip = true;
    Args.bLockable = false;
    Args.bUpdatesFromSelection = false;
    Args.NameAreaSettings = FDetailsViewArgs::HideNameArea;
    FPropertyEditorModule& PropertyEditor = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    const TSharedRef<IDetailsView> Details = PropertyEditor.CreateDetailView(Args);
    Details->SetObject(Settings.Get());

    return SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(8)
        [
            SNew(STextBlock)
            .Text(FText::FromString(TEXT("Stage Set Actor Exporter")))
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(8, 0, 8, 8)
        [
            SNew(STextBlock).AutoWrapText(true)
            .Text(FText::FromString(TEXT("Exports loaded editor Actors with a StageSetExportComponent. Transform uses WORLD space. Replace All removes every existing row; use a dedicated table.")))
        ]
        + SVerticalBox::Slot().FillHeight(1.0f).Padding(8)
        [ Details ]
        + SVerticalBox::Slot().AutoHeight().Padding(8, 4)
        [
            SNew(SButton).Text(FText::FromString(TEXT("Assign missing RowNames (Actor_001, ...)")))
            .OnClicked_UObject(this, &UStageSetExportWidget::OnAssignNamesClicked)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(8, 4)
        [
            SNew(SButton).Text(FText::FromString(TEXT("Validate Actors")))
            .OnClicked_UObject(this, &UStageSetExportWidget::OnValidateClicked)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(8, 4)
        [
            SNew(SButton).Text(FText::FromString(TEXT("Export Actors to Table")))
            .OnClicked_UObject(this, &UStageSetExportWidget::OnExportClicked)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(8, 4)
        [
            SNew(SButton).Text(FText::FromString(TEXT("Save Target Table")))
            .OnClicked_UObject(this, &UStageSetExportWidget::OnSaveClicked)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(8)
        [
            SNew(STextBlock).AutoWrapText(true)
            .Text_UObject(this, &UStageSetExportWidget::GetStatusText)
        ];
}

bool UStageSetExportWidget::CollectActors(TArray<AActor*>& OutActors, FString& OutError) const
{
    OutActors.Reset();
    if (!GEditor || GEditor->PlayWorld || !Settings)
    {
        OutError = TEXT("Run this tool in the level editor, outside PIE/Simulate.");
        return false;
    }
    UWorld* World = GEditor->GetEditorWorldContext().World();
    UEditorActorSubsystem* Actors = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
    if (!World || World->WorldType != EWorldType::Editor || !Actors)
    {
        OutError = TEXT("No valid editor world is open.");
        return false;
    }

    const TArray<AActor*> Candidates = Settings->bOnlySelectedActors
        ? Actors->GetSelectedLevelActors() : Actors->GetAllLevelActors();
    for (AActor* Actor : Candidates)
    {
        if (!IsValid(Actor) || Actor->IsTemplate() || Actor->IsActorBeingDestroyed() || Actor->GetWorld() != World) continue;
        if (Settings->ActorClass && !Actor->IsA(Settings->ActorClass.Get())) continue;
        if (!Settings->RequiredActorTag.IsNone() && !Actor->ActorHasTag(Settings->RequiredActorTag)) continue;
        if (Settings->bOnlyCurrentLevel && Actor->GetLevel() != World->GetCurrentLevel()) continue;
        if (!Actor->FindComponentByClass<UStageSetExportComponent>()) continue;
        OutActors.Add(Actor);
    }
    // Deterministic assignment order; generated names are then persisted on Actors.
    OutActors.Sort([](const AActor& Left, const AActor& Right)
    {
        return Left.GetPathName() < Right.GetPathName();
    });
    if (OutActors.IsEmpty())
    {
        OutError = TEXT("No matching loaded Actors with a StageSetExportComponent. Nothing was changed.");
        return false;
    }
    return true;
}

bool UStageSetExportWidget::ValidateCollectedActors(const TArray<AActor*>& Actors, FString& OutError) const
{
    TMap<FName, AActor*> Names;
    TArray<FString> Errors;
    for (AActor* Actor : Actors)
    {
        TArray<UStageSetExportComponent*> Components;
        Actor->GetComponents<UStageSetExportComponent>(Components);
        if (Components.Num() != 1)
        {
            Errors.Add(FString::Printf(TEXT("%s: expected exactly one StageSetExportComponent."), *Actor->GetActorLabel()));
            continue;
        }
        const FName Name = Components[0]->RowName;
        if (Name.IsNone())
            Errors.Add(FString::Printf(TEXT("%s: RowName is empty. Assign a name first."), *Actor->GetActorLabel()));
        else if (AActor** Other = Names.Find(Name))
            Errors.Add(FString::Printf(TEXT("Duplicate RowName %s: %s and %s."),
                *Name.ToString(), *(*Other)->GetActorLabel(), *Actor->GetActorLabel()));
        else
            Names.Add(Name, Actor);

        if (!Actor->GetActorTransform().IsValid())
            Errors.Add(FString::Printf(TEXT("%s: invalid Transform."), *Actor->GetActorLabel()));
    }
    // Catch duplicated source keys even when the other Actor is outside the filter.
    if (GEditor)
    {
        if (UEditorActorSubsystem* Subsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>())
        {
            for (AActor* OtherActor : Subsystem->GetAllLevelActors())
            {
                if (!IsValid(OtherActor) || Actors.Contains(OtherActor)) continue;
                TArray<UStageSetExportComponent*> Components;
                OtherActor->GetComponents<UStageSetExportComponent>(Components);
                for (const UStageSetExportComponent* Component : Components)
                {
                    if (AActor* const* ExportedActor = Names.Find(Component->RowName))
                    {
                        Errors.Add(FString::Printf(TEXT("RowName %s is also used outside the filter by %s (exported Actor: %s)."),
                            *Component->RowName.ToString(), *OtherActor->GetActorLabel(), *(*ExportedActor)->GetActorLabel()));
                    }
                }
            }
        }
    }
    OutError = FString::Join(Errors, TEXT("\n"));
    return Errors.IsEmpty();
}

bool UStageSetExportWidget::ValidateActors()
{
    TArray<AActor*> Actors;
    FString Error;
    if (!CollectActors(Actors, Error) || !ValidateCollectedActors(Actors, Error))
    {
        SetStatus(Error, true);
        return false;
    }
    SetStatus(FString::Printf(TEXT("Validation passed: %d Actors. Only currently loaded Actors were inspected."), Actors.Num()));
    return true;
}

bool UStageSetExportWidget::AssignMissingRowNames()
{
    TArray<AActor*> Actors;
    FString Error;
    if (!CollectActors(Actors, Error))
    {
        SetStatus(Error, true);
        return false;
    }

    // Avoid names from all loaded exportable Actors, even outside the current filter.
    TSet<FName> UsedNames;
    UEditorActorSubsystem* Subsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
    for (AActor* Actor : Subsystem->GetAllLevelActors())
    {
        TArray<UStageSetExportComponent*> Components;
        Actor->GetComponents<UStageSetExportComponent>(Components);
        for (const UStageSetExportComponent* Component : Components)
            if (!Component->RowName.IsNone()) UsedNames.Add(Component->RowName);
    }
    if (Settings->TargetTable)
        for (const FName Name : Settings->TargetTable->GetRowNames()) UsedNames.Add(Name);

    // Validate the component count before mutating any Actor.
    for (AActor* Actor : Actors)
    {
        TArray<UStageSetExportComponent*> Components;
        Actor->GetComponents<UStageSetExportComponent>(Components);
        if (Components.Num() != 1)
        {
            SetStatus(FString::Printf(TEXT("%s: keep exactly one StageSetExportComponent."), *Actor->GetActorLabel()), true);
            return false;
        }
    }

    int32 Assigned = 0;
    int32 Number = 1;
    {
        const FScopedTransaction Transaction(NSLOCTEXT("StageSetExporter", "AssignNames", "Assign Stage Set Row Names"));
        for (AActor* Actor : Actors)
        {
            UStageSetExportComponent* Component = Actor->FindComponentByClass<UStageSetExportComponent>();
            if (!Component->RowName.IsNone()) continue;
            FName Name;
            do { Name = FName(*FString::Printf(TEXT("Actor_%03d"), Number++)); }
            while (UsedNames.Contains(Name));
            Actor->Modify();
            Component->Modify();
            Component->RowName = Name;
            Actor->MarkPackageDirty();
            UsedNames.Add(Name);
            ++Assigned;
        }
    }
    SetStatus(FString::Printf(TEXT("Assigned %d missing RowNames. Save the affected levels/external Actors. Existing names and integer Id values were not changed."), Assigned));
    return true;
}

bool UStageSetExportWidget::ExportActors()
{
    TArray<AActor*> Actors;
    FString Error;
    if (!CollectActors(Actors, Error) || !ValidateCollectedActors(Actors, Error))
    {
        SetStatus(Error, true);
        return false;
    }
    UStageSetTable* Table = Settings->TargetTable.Get();
    if (!IsValid(Table) || Table->GetRowStruct() != FStageSetRow::StaticStruct() ||
        Table->HasAnyFlags(RF_Transient) || Table->GetOutermost() == GetTransientPackage())
    {
        SetStatus(TEXT("Select a saved StageSetTable asset with FStageSetRow as its Row Structure."), true);
        return false;
    }
    if (Settings->bOnlySelectedActors && Settings->ExportMode == EStageSetExportMode::ReplaceAll)
    {
        SetStatus(TEXT("Replace All is disabled for selected-only export. Use Upsert, or disable Only Selected Actors."), true);
        return false;
    }

    // Build the complete output before touching the destination.
    TMap<FName, FStageSetRow> Rows;
    for (AActor* Actor : Actors)
    {
        const UStageSetExportComponent* Component = Actor->FindComponentByClass<UStageSetExportComponent>();
        Rows.Add(Component->RowName, StageSetExporter::MakeRow(*Actor, *Component));
    }
    {
        const FScopedTransaction Transaction(NSLOCTEXT("StageSetExporter", "ExportActors", "Export Stage Set Actors"));
        FDataTableEditorUtils::BroadcastPreChange(Table, FDataTableEditorUtils::EDataTableChangeInfo::RowList);
        Table->Modify();
        if (Settings->ExportMode == EStageSetExportMode::ReplaceAll) Table->EmptyTable();
        TArray<FName> OrderedNames;
        Rows.GetKeys(OrderedNames);
        OrderedNames.Sort([](FName Left, FName Right) { return Left.LexicalLess(Right); });
        for (const FName Name : OrderedNames) Table->AddRow(Name, Rows.FindChecked(Name));
        Table->MarkPackageDirty();
        FDataTableEditorUtils::BroadcastPostChange(Table, FDataTableEditorUtils::EDataTableChangeInfo::RowList);
    }

    if (Settings->bSaveAfterExport)
    {
        UEditorAssetSubsystem* AssetSubsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
        if (!AssetSubsystem || !AssetSubsystem->SaveLoadedAsset(Table, false))
        {
            SetStatus(FString::Printf(TEXT("Exported %d rows in memory, but saving FAILED. Review the table and save it manually."), Rows.Num()), true);
            return false;
        }
    }
    SetStatus(FString::Printf(TEXT("Exported %d rows to %s. Table now has %d rows. %s"),
        Rows.Num(), *Table->GetName(), Table->GetRowMap().Num(),
        Settings->bSaveAfterExport ? TEXT("Saved to disk.") : TEXT("Not saved; use Save Target Table.")));
    return true;
}

bool UStageSetExportWidget::SaveTargetTable()
{
    if (!GEditor || GEditor->PlayWorld || !Settings || !IsValid(Settings->TargetTable.Get()))
    {
        SetStatus(TEXT("Select a StageSetTable and stop PIE/Simulate before saving."), true);
        return false;
    }
    UEditorAssetSubsystem* Subsystem = GEditor->GetEditorSubsystem<UEditorAssetSubsystem>();
    const bool bSaved = Subsystem && Subsystem->SaveLoadedAsset(Settings->TargetTable.Get(), false);
    SetStatus(bSaved ? TEXT("Target table saved.") : TEXT("Target table save failed. See Output Log."), !bSaved);
    return bSaved;
}

void UStageSetExportWidget::SetStatus(const FString& Message, bool bError)
{
    StatusText = FText::FromString(Message);
    if (bError)
    {
        UE_LOG(LogStageSetExporter, Error, TEXT("%s"), *Message);
    }
    else
    {
        UE_LOG(LogStageSetExporter, Display, TEXT("%s"), *Message);
    }
}

FText UStageSetExportWidget::GetStatusText() const
{
    return StatusText;
}

FReply UStageSetExportWidget::OnValidateClicked() { ValidateActors(); return FReply::Handled(); }
FReply UStageSetExportWidget::OnExportClicked() { ExportActors(); return FReply::Handled(); }
FReply UStageSetExportWidget::OnAssignNamesClicked() { AssignMissingRowNames(); return FReply::Handled(); }
FReply UStageSetExportWidget::OnSaveClicked() { SaveTargetTable(); return FReply::Handled(); }
