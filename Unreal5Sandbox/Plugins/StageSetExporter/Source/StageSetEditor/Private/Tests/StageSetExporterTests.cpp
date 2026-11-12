#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Components/SceneComponent.h"
#include "Editor.h"
#include "EditorUtilityWidgetBlueprint.h"
#include "EditorUtilityWidgetBlueprintFactory.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/ScopeExit.h"
#include "StageSetExportComponent.h"
#include "StageSetExportWidget.h"
#include "StageSetTable.h"
#include "StageSetTableFactory.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStageSetWidgetBlueprintTest,
    "StageSetExporter.EditorUtilityWidgetCreation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStageSetWidgetBlueprintTest::RunTest(const FString& Parameters)
{
    UClass* WidgetClass = UStageSetExportWidget::StaticClass();
    TestTrue(TEXT("Native class can be a Blueprint parent"), FKismetEditorUtilities::CanCreateBlueprintOfClass(WidgetClass));
    TestTrue(TEXT("Correct Editor Utility Widget inheritance"), WidgetClass->IsChildOf(UEditorUtilityWidget::StaticClass()));

    TStrongObjectPtr<UEditorUtilityWidgetBlueprintFactory> Factory(NewObject<UEditorUtilityWidgetBlueprintFactory>());
    Factory->ParentClass = WidgetClass;
    Factory->RootWidgetClass = nullptr;
    const FName Name = MakeUniqueObjectName(GetTransientPackage(), UEditorUtilityWidgetBlueprint::StaticClass(), TEXT("StageSetTestWidget"));
    UEditorUtilityWidgetBlueprint* Blueprint = Cast<UEditorUtilityWidgetBlueprint>(Factory->FactoryCreateNew(
        UEditorUtilityWidgetBlueprint::StaticClass(), GetTransientPackage(), Name, RF_Transient, nullptr, GWarn));
    if (!TestNotNull(TEXT("Stock UE factory creates the utility Blueprint"), Blueprint)) return false;
    TStrongObjectPtr<UEditorUtilityWidgetBlueprint> BlueprintGuard(Blueprint);
    FKismetEditorUtilities::CompileBlueprint(Blueprint);
    TestTrue(TEXT("Blueprint compiled without errors"), Blueprint->Status != BS_Error);
    if (!TestNotNull(TEXT("Blueprint has a generated class"), Blueprint->GeneratedClass.Get())) return false;

    TStrongObjectPtr<UStageSetExportWidget> Widget(NewObject<UStageSetExportWidget>(
        GetTransientPackage(), Blueprint->GeneratedClass));
    Widget->Initialize();
    const TSharedRef<SWidget> SlateWidget = Widget->TakeWidget();
    TestNotNull(TEXT("Native UI created its export settings"), Widget->Settings.Get());
    Widget->ReleaseSlateResources(true);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStageSetExportFlowTest,
    "StageSetExporter.ExportAndDuplicateValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FStageSetExportFlowTest::RunTest(const FString& Parameters)
{
    if (!TestNotNull(TEXT("Editor engine"), GEditor)) return false;
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!TestNotNull(TEXT("Editor world"), World)) return false;

    UPackage* LevelPackage = World->GetCurrentLevel()->GetOutermost();
    const bool bLevelWasDirty = LevelPackage->IsDirty();
    UPackage* TablePackage = CreatePackage(*FString::Printf(TEXT("/Game/StageSetTest_%s"), *FGuid::NewGuid().ToString(EGuidFormats::Digits)));
    TArray<AActor*> TestActors;
    ON_SCOPE_EXIT
    {
        for (AActor* Actor : TestActors) if (IsValid(Actor)) World->DestroyActor(Actor);
        TablePackage->SetDirtyFlag(false);
        LevelPackage->SetDirtyFlag(bLevelWasDirty);
    };

    TStrongObjectPtr<UStageSetTableFactory> Factory(NewObject<UStageSetTableFactory>());
    TStrongObjectPtr<UStageSetTable> Table(Cast<UStageSetTable>(Factory->FactoryCreateNew(
        UStageSetTable::StaticClass(), TablePackage, TEXT("DT_Test"), RF_Transactional, nullptr, GWarn)));
    if (!TestNotNull(TEXT("Custom factory creates StageSetTable"), Table.Get())) return false;
    TestTrue(TEXT("Correct row structure"), Table->GetRowStruct() == FStageSetRow::StaticStruct());

    TStrongObjectPtr<UStageSetExportWidget> Widget(NewObject<UStageSetExportWidget>());
    Widget->Settings = NewObject<UStageSetExportSettings>(Widget.Get());
    Widget->Settings->TargetTable = Table.Get();
    Widget->Settings->bSaveAfterExport = false;
    Widget->Settings->RequiredActorTag = FName(*FGuid::NewGuid().ToString(EGuidFormats::Digits));
    Widget->Settings->ExportMode = EStageSetExportMode::Upsert;

    UStageSetExportComponent* Sources[2] = {};
    for (int32 Index = 0; Index < 2; ++Index)
    {
        FActorSpawnParameters Spawn;
        // EditorActorSubsystem intentionally excludes RF_Transient actors.
        // These fixtures are destroyed on scope exit and their map is never saved.
        Spawn.ObjectFlags = RF_Transactional;
        AActor* Actor = World->SpawnActor<AActor>(Spawn);
        if (!TestNotNull(TEXT("Test source Actor"), Actor)) return false;
        TestActors.Add(Actor);
        USceneComponent* Root = NewObject<USceneComponent>(Actor);
        Actor->AddInstanceComponent(Root);
        Actor->SetRootComponent(Root);
        Root->RegisterComponent();
        Actor->SetActorTransform(FTransform(FRotator(0, 45 + Index * 30, 0), FVector(100 + Index * 200, 300, 400), FVector(1 + Index)));
        Actor->Tags.Add(Widget->Settings->RequiredActorTag);
        UStageSetExportComponent* Source = NewObject<UStageSetExportComponent>(Actor);
        Actor->AddInstanceComponent(Source);
        Source->RegisterComponent();
        Source->RowName = Index == 0 ? FName(TEXT("Actor_001")) : FName(TEXT("Actor_002"));
        Source->Category = 7;
        Source->Id = Index + 10;
        Source->AreaId = 2;
        Source->Param01 = 11;
        Source->Param02 = 22;
        Source->Param03 = 33;
        Source->Param04 = 44;
        Sources[Index] = Source;
    }

    TestTrue(TEXT("Export two Actors"), Widget->ExportActors());
    TestEqual(TEXT("Exactly two rows"), Table->GetRowMap().Num(), 2);
    const FStageSetRow* Row = Table->FindRow<FStageSetRow>(TEXT("Actor_002"), TEXT("Automation"));
    if (!TestNotNull(TEXT("Second exported row"), Row)) return false;
    TestTrue(TEXT("World transform including rotation and scale"), Row->Transform.Equals(TestActors[1]->GetActorTransform()));
    TestEqual(TEXT("Category exported"), Row->Category, 7);
    TestEqual(TEXT("Id exported"), Row->Id, 11);
    TestEqual(TEXT("AreaId exported"), Row->AreaId, 2);
    TestTrue(TEXT("All parameters exported"), Row->Param01 == 11 && Row->Param02 == 22 && Row->Param03 == 33 && Row->Param04 == 44);

    Sources[1]->Param04 = 99;
    TestTrue(TEXT("Repeated export updates rows"), Widget->ExportActors());
    TestEqual(TEXT("Upsert does not duplicate rows"), Table->GetRowMap().Num(), 2);
    TestEqual(TEXT("Updated parameter"), Table->FindRow<FStageSetRow>(TEXT("Actor_002"), TEXT("Automation"))->Param04, 99);

    Sources[1]->RowName = Sources[0]->RowName;
    AddExpectedError(TEXT("Duplicate RowName"), EAutomationExpectedErrorFlags::Contains, 1);
    TestFalse(TEXT("Duplicate export rejected"), Widget->ExportActors());
    TestEqual(TEXT("Duplicate failure preserves destination"), Table->GetRowMap().Num(), 2);
    TestEqual(TEXT("Existing row data preserved"), Table->FindRow<FStageSetRow>(TEXT("Actor_002"), TEXT("Automation"))->Param04, 99);

    Widget->Settings->RequiredActorTag = FName(TEXT("NoMatchingStageSetTestActor"));
    Widget->Settings->ExportMode = EStageSetExportMode::ReplaceAll;
    AddExpectedError(TEXT("No matching loaded Actors"), EAutomationExpectedErrorFlags::Contains, 1);
    TestFalse(TEXT("Empty ReplaceAll rejected"), Widget->ExportActors());
    TestEqual(TEXT("Empty export preserves destination"), Table->GetRowMap().Num(), 2);
    return true;
}

#endif
