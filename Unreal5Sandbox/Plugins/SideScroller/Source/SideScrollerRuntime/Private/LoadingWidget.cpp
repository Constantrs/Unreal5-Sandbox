#include "LoadingWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"

void ULoadingWidget::SetProgress(float InProgress)
{
    Progress = FMath::IsFinite(InProgress) ? FMath::Clamp(InProgress, 0.0f, 1.0f) : 0.0f;
    if (ProgressBar) ProgressBar->SetPercent(Progress);
}

TSharedRef<SWidget> ULoadingWidget::RebuildWidget()
{
    if (!WidgetTree) WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"));
    if (!ProgressBar)
    {
        UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("LoadingCanvas"));
        WidgetTree->RootWidget = Canvas;
        ProgressBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("LoadingProgressBar"));
        ProgressBar->SetIsMarquee(false);
        ProgressBar->SetFillColorAndOpacity(FLinearColor(0.2f, 0.65f, 1.0f));
        UCanvasPanelSlot* BarSlot = Canvas->AddChildToCanvas(ProgressBar);
        BarSlot->SetAnchors(FAnchors(0.5f, 0.85f));
        BarSlot->SetAlignment(FVector2D(0.5f, 0.5f));
        BarSlot->SetPosition(FVector2D::ZeroVector);
        BarSlot->SetSize(FVector2D(600.0f, 24.0f));
    }
    ProgressBar->SetPercent(Progress);
    // This UI does not take focus or consume player input.
    SetVisibility(ESlateVisibility::HitTestInvisible);
    return Super::RebuildWidget();
}
