#include "UI/DMTextPanelWidget.h"

#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanel.h"
#include "Blueprint/WidgetTree.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UDMTextPanelWidget::NativeConstruct()
{
    Super::NativeConstruct();
    EnsureFallbackWidgets();
}

void UDMTextPanelWidget::PushNarration(const FString& NarrationText)
{
    if (!NarrationTextBlock)
    {
        return;
    }

    FullText = NarrationText;
    TypewriterIndex = 0;
    NarrationTextBlock->SetText(FText::GetEmpty());

    if (UWorld* World = GetWorld())
    {
        const float TickRate = FMath::Max(0.01f, 1.0f / FMath::Max(1.0f, TypewriterCharsPerSecond));
        World->GetTimerManager().SetTimer(TypewriterTimer, this, &UDMTextPanelWidget::AdvanceTypewriter, TickRate, true);
    }
}

void UDMTextPanelWidget::EnsureFallbackWidgets()
{
    if (NarrationScrollBox && NarrationTextBlock)
    {
        return;
    }

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("DMPanelRoot"));
    WidgetTree->RootWidget = Root;

    NarrationScrollBox = WidgetTree->ConstructWidget<UScrollBox>(UScrollBox::StaticClass(), TEXT("NarrationScroll"));
    NarrationTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NarrationText"));

    NarrationTextBlock->SetColorAndOpacity(FSlateColor(FLinearColor(0.94f, 0.90f, 0.83f, 1.0f)));
    NarrationScrollBox->AddChild(NarrationTextBlock);
    Root->AddChild(NarrationScrollBox);
}

void UDMTextPanelWidget::AdvanceTypewriter()
{
    if (!NarrationTextBlock)
    {
        return;
    }

    TypewriterIndex = FMath::Min(TypewriterIndex + 1, FullText.Len());
    NarrationTextBlock->SetText(FText::FromString(FullText.Left(TypewriterIndex)));

    if (NarrationScrollBox)
    {
        NarrationScrollBox->ScrollToEnd();
    }

    if (TypewriterIndex >= FullText.Len())
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(TypewriterTimer);
        }
    }
}
