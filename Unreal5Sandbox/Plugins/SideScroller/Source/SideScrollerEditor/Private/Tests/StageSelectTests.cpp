#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "SideScrollerStageSelectWidget.h"
#include "SideScrollerStageTable.h"
#include "UObject/StrongObjectPtr.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSideScrollerStageSelectTest,
    "SideScroller.StageSelect.PaginationAndDataBinding",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSideScrollerStageSelectTest::RunTest(const FString& Parameters)
{
    TStrongObjectPtr<USideScrollerStageTable> Table(NewObject<USideScrollerStageTable>());
    // Insert in reverse order to verify the menu does not depend on map iteration order.
    for (int32 Number = 25; Number >= 1; --Number)
    {
        FSideScrollerStageRow Row;
        Row.StageName = FName(*FString::Printf(TEXT("Stage %02d"), Number));
        Row.LevelName = FString::Printf(TEXT("/Game/Maps/L_Stage%02d"), Number);
        Table->AddRow(FName(*FString::Printf(TEXT("Stage%03d"), Number)), Row);
    }
    TStrongObjectPtr<USideScrollerStageSelectWidget> Widget(NewObject<USideScrollerStageSelectWidget>());
    Widget->Initialize();
    TestTrue(TEXT("Load stage data before constructing UI"), Widget->SetStageTable(Table.Get()));
    const TSharedRef<SWidget> SlateWidget = Widget->TakeWidget();
    UVerticalBox* List = Cast<UVerticalBox>(Widget->GetWidgetFromName(TEXT("StageList")));
    if (!TestNotNull(TEXT("Native UMG VerticalBox exists"), List)) return false;
    TestEqual(TEXT("20 row widgets per page"), List->GetChildrenCount(), 20);
    TestEqual(TEXT("25 stages, two pages"), Widget->GetPageCount(), 2);
    TestEqual(TEXT("Starts with first row"), Widget->GetSelectedIndex(), 0);
    Widget->MoveSelection(-1);
    TestEqual(TEXT("Up at first row remains at first row"), Widget->GetSelectedIndex(), 0);
    for (int32 Step = 0; Step < 20; ++Step) Widget->MoveSelection(1);
    TestEqual(TEXT("Down across page boundary"), Widget->GetCurrentPageIndex(), 1);
    FSideScrollerStageRow Selected;
    TestTrue(TEXT("Selected row available"), Widget->GetSelectedStage(Selected));
    TestEqual(TEXT("Correct sorted stage selected"), Selected.StageName, FName(TEXT("Stage 21")));
    TestEqual(TEXT("OpenLevel target follows selected row"), Selected.LevelName, FString(TEXT("/Game/Maps/L_Stage21")));
    UTextBlock* First = Cast<UTextBlock>(List->GetChildAt(0));
    TestTrue(TEXT("Second-page cursor and name rendered"), First && First->GetText().ToString().Contains(TEXT(">  Stage 21")));
    UTextBlock* Blank = Cast<UTextBlock>(List->GetChildAt(5));
    TestTrue(TEXT("Unused last-page slot blank"), Blank && Blank->GetText().IsEmpty());
    Widget->MoveSelection(-1);
    TestEqual(TEXT("Up crosses back to page one"), Widget->GetCurrentPageIndex(), 0);
    for (int32 Step = 0; Step < 100; ++Step) Widget->MoveSelection(1);
    TestEqual(TEXT("Selection stops at last row"), Widget->GetSelectedIndex(), 24);
    TestFalse(TEXT("Invalid table rejected"), Widget->SetStageTable(nullptr));
    TestEqual(TEXT("Invalid data clears selection"), Widget->GetSelectedIndex(), INDEX_NONE);
    TestFalse(TEXT("No stage selected after invalid data"), Widget->GetSelectedStage(Selected));
    Table->EmptyTable();
    TestTrue(TEXT("Empty valid table supported"), Widget->SetStageTable(Table.Get()));
    Widget->MoveSelection(1);
    TestEqual(TEXT("Empty table has zero pages"), Widget->GetPageCount(), 0);
    Widget->ReleaseSlateResources(true);
    return true;
}

#endif
