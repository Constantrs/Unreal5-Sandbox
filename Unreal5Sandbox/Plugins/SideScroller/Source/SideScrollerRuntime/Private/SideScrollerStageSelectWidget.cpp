#include "SideScrollerStageSelectWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

#define LOCTEXT_NAMESPACE "SideScrollerStageSelect"

bool USideScrollerStageSelectWidget::SetStageTable(UDataTable* InTable)
{
    StageTable = nullptr;
    Stages.Reset();
    SelectedIndex = INDEX_NONE;
    if (!IsValid(InTable) || InTable->GetRowStruct() != FSideScrollerStageRow::StaticStruct())
    {
        SetStatusMessage(LOCTEXT("InvalidTable", "PlayerControllerにステージのDataTableを設定してください"));
        RefreshPage();
        return false;
    }

    StageTable = InTable;
    TArray<FName> RowNames = InTable->GetRowNames();
    // A DataTable's map iteration order is not a reliable menu order.
    RowNames.Sort([](FName Left, FName Right) { return Left.LexicalLess(Right); });
    for (const FName RowName : RowNames)
    {
        if (const FSideScrollerStageRow* Source = InTable->FindRow<FSideScrollerStageRow>(RowName, TEXT("StageSelect")))
        {
            FSideScrollerStageRow Row = *Source;
            if (Row.StageName.IsNone()) Row.StageName = RowName;
            Stages.Add(MoveTemp(Row));
        }
    }
    SelectedIndex = Stages.IsEmpty() ? INDEX_NONE : 0;
    SetStatusMessage(Stages.IsEmpty() ? LOCTEXT("Empty", "選択できるステージがありません") : FText::GetEmpty());
    RefreshPage();
    return true;
}

void USideScrollerStageSelectWidget::MoveSelection(int32 Direction)
{
    if (Stages.IsEmpty() || Direction == 0) return;
    SelectedIndex = FMath::Clamp(SelectedIndex + (Direction > 0 ? 1 : -1), 0, Stages.Num() - 1);
    SetStatusMessage(FText::GetEmpty());
    RefreshPage();
}

bool USideScrollerStageSelectWidget::GetSelectedStage(FSideScrollerStageRow& OutStage) const
{
    OutStage = FSideScrollerStageRow();
    if (!Stages.IsValidIndex(SelectedIndex)) return false;
    OutStage = Stages[SelectedIndex];
    return true;
}

int32 USideScrollerStageSelectWidget::GetPageCount() const
{
    return FMath::DivideAndRoundUp(Stages.Num(), StagesPerPage);
}

int32 USideScrollerStageSelectWidget::GetCurrentPageIndex() const
{
    return SelectedIndex == INDEX_NONE ? INDEX_NONE : SelectedIndex / StagesPerPage;
}

void USideScrollerStageSelectWidget::SetStatusMessage(const FText& Message)
{
    StatusMessage = Message;
    if (StatusLabel) StatusLabel->SetText(Message);
}

TSharedRef<SWidget> USideScrollerStageSelectWidget::RebuildWidget()
{
    BuildWidgetTree();
    RefreshPage();
    return Super::RebuildWidget();
}

void USideScrollerStageSelectWidget::BuildWidgetTree()
{
    if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
    // Reuse the tree when Slate resources are rebuilt.
    if (StageList && StageLabels.Num() == StagesPerPage) return;

    UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Background"));
    Background->SetBrushColor(FLinearColor(0.025f, 0.035f, 0.06f, 1.0f));
    Background->SetPadding(FMargin(60.0f, 24.0f));
    WidgetTree->RootWidget = Background;
    UVerticalBox* Layout = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Layout"));
    Background->SetContent(Layout);

    UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Title"));
    Title->SetText(LOCTEXT("Title", "ステージ選択"));
    FSlateFontInfo TitleFont = Title->GetFont();
    TitleFont.Size = 28;
    Title->SetFont(TitleFont);
    Layout->AddChildToVerticalBox(Title)->SetPadding(FMargin(0, 0, 0, 8));

    StageList = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StageList"));
    Layout->AddChildToVerticalBox(StageList)->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    StageLabels.Reset();
    for (int32 Index = 0; Index < StagesPerPage; ++Index)
    {
        UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),
            FName(*FString::Printf(TEXT("StageLabel_%02d"), Index)));
        FSlateFontInfo Font = Label->GetFont();
        Font.Size = 18;
        Label->SetFont(Font);
        UVerticalBoxSlot* RowSlot = StageList->AddChildToVerticalBox(Label);
        RowSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        RowSlot->SetVerticalAlignment(VAlign_Center);
        StageLabels.Add(Label);
    }
    PageLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PageLabel"));
    Layout->AddChildToVerticalBox(PageLabel)->SetPadding(FMargin(0, 8, 0, 4));

    UTextBlock* Help = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("Help"));
    Help->SetText(LOCTEXT("Help", "↑ / ↓ カーソル移動    Enter 決定    Esc 終了"));
    Layout->AddChildToVerticalBox(Help);
    StatusLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("StatusLabel"));
    StatusLabel->SetAutoWrapText(true);
    StatusLabel->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.5f, 0.3f)));
    Layout->AddChildToVerticalBox(StatusLabel);
}

void USideScrollerStageSelectWidget::RefreshPage()
{
    if (!StageList) return;
    const int32 PageIndex = GetCurrentPageIndex();
    const int32 First = PageIndex == INDEX_NONE ? 0 : PageIndex * StagesPerPage;
    for (int32 RowIndex = 0; RowIndex < StageLabels.Num(); ++RowIndex)
    {
        const int32 Index = First + RowIndex;
        UTextBlock* Label = StageLabels[RowIndex];
        if (!Stages.IsValidIndex(Index))
        {
            Label->SetText(FText::GetEmpty());
            continue;
        }
        const bool bSelected = Index == SelectedIndex;
        Label->SetText(FText::Format(LOCTEXT("StageItem", "{0}{1}"),
            FText::FromString(bSelected ? TEXT(">  ") : TEXT("   ")), FText::FromName(Stages[Index].StageName)));
        Label->SetColorAndOpacity(FSlateColor(bSelected ? FLinearColor::Yellow : FLinearColor::White));
    }
    PageLabel->SetText(FText::Format(LOCTEXT("Page", "現ページ {0} / {1}    ステージ総数 {2}"),
        FText::AsNumber(PageIndex == INDEX_NONE ? 0 : PageIndex + 1),
        FText::AsNumber(GetPageCount()), FText::AsNumber(Stages.Num())));
    StatusLabel->SetText(StatusMessage);
}

#undef LOCTEXT_NAMESPACE
