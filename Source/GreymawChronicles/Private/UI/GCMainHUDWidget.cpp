#include "UI/GCMainHUDWidget.h"

#include "DungeonMaster/DMBrainSubsystem.h"
#include "UI/DMTextPanelWidget.h"
#include "UI/GCDebugOverlayWidget.h"
#include "UI/GCHUDOverlayWidget.h"
#include "UI/GCActionFeedbackWidget.h"
#include "UI/GCPlayerInputWidget.h"
#include "UI/GCQuestJournalWidget.h"
#include "UI/GCSaveLoadWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"

void UGCMainHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();
    EnsureFallbackWidgets();

    if (UGameInstance* GI = GetGameInstance())
    {
        DMBrain = GI->GetSubsystem<UDMBrainSubsystem>();
        if (DMBrain.IsValid())
        {
            DMBrain->OnDMNarration.AddDynamic(this, &UGCMainHUDWidget::HandleNarration);
        }
    }
}

void UGCMainHUDWidget::HandleNarration(const FString& NarrationText)
{
    if (DMTextPanel)
    {
        DMTextPanel->PushNarration(NarrationText);
    }
}

void UGCMainHUDWidget::EnsureFallbackWidgets()
{
    if (DMTextPanel && PlayerInput && HUDOverlay)
    {
        return;
    }

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("MainHUDRoot"));
    WidgetTree->RootWidget = Root;

    HUDOverlay = WidgetTree->ConstructWidget<UGCHUDOverlayWidget>(UGCHUDOverlayWidget::StaticClass(), TEXT("HUDOverlay"));
    DMTextPanel = WidgetTree->ConstructWidget<UDMTextPanelWidget>(UDMTextPanelWidget::StaticClass(), TEXT("DMTextPanel"));
    PlayerInput = WidgetTree->ConstructWidget<UGCPlayerInputWidget>(UGCPlayerInputWidget::StaticClass(), TEXT("PlayerInput"));
    DebugOverlay = WidgetTree->ConstructWidget<UGCDebugOverlayWidget>(UGCDebugOverlayWidget::StaticClass(), TEXT("DebugOverlay"));
    ActionFeedback = WidgetTree->ConstructWidget<UGCActionFeedbackWidget>(UGCActionFeedbackWidget::StaticClass(), TEXT("ActionFeedback"));

    if (UCanvasPanelSlot* OverlaySlot = Root->AddChildToCanvas(HUDOverlay))
    {
        OverlaySlot->SetAutoSize(true);
        OverlaySlot->SetPosition(FVector2D(20.0f, 20.0f));
    }

    // Debug overlay — top-right corner
    if (UCanvasPanelSlot* DebugSlot = Root->AddChildToCanvas(DebugOverlay))
    {
        DebugSlot->SetAutoSize(true);
        DebugSlot->SetPosition(FVector2D(800.0f, 20.0f));
    }

    // Action feedback toasts — center-right
    if (UCanvasPanelSlot* FeedbackSlot = Root->AddChildToCanvas(ActionFeedback))
    {
        FeedbackSlot->SetAutoSize(true);
        FeedbackSlot->SetPosition(FVector2D(860.0f, 300.0f));
    }

    if (UCanvasPanelSlot* DMPanelSlot = Root->AddChildToCanvas(DMTextPanel))
    {
        DMPanelSlot->SetAutoSize(false);
        DMPanelSlot->SetPosition(FVector2D(40.0f, 620.0f));
        DMPanelSlot->SetSize(FVector2D(1200.0f, 160.0f));
    }

    if (UCanvasPanelSlot* InputSlot = Root->AddChildToCanvas(PlayerInput))
    {
        InputSlot->SetAutoSize(false);
        InputSlot->SetPosition(FVector2D(40.0f, 790.0f));
        InputSlot->SetSize(FVector2D(1200.0f, 100.0f));
    }

    // Sprint L: Quest journal panel (hidden by default)
    QuestJournal = WidgetTree->ConstructWidget<UGCQuestJournalWidget>(UGCQuestJournalWidget::StaticClass(), TEXT("QuestJournal"));
    if (UCanvasPanelSlot* QuestSlot = Root->AddChildToCanvas(QuestJournal))
    {
        QuestSlot->SetAutoSize(false);
        QuestSlot->SetPosition(FVector2D(40.0f, 100.0f));
        QuestSlot->SetSize(FVector2D(400.0f, 500.0f));
    }
    if (QuestJournal)
    {
        QuestJournal->SetVisibility(ESlateVisibility::Collapsed);
    }

    // Sprint L: Save/load panel (hidden by default)
    SaveLoadPanel = WidgetTree->ConstructWidget<UGCSaveLoadWidget>(UGCSaveLoadWidget::StaticClass(), TEXT("SaveLoadPanel"));
    if (UCanvasPanelSlot* SaveSlot = Root->AddChildToCanvas(SaveLoadPanel))
    {
        SaveSlot->SetAutoSize(false);
        SaveSlot->SetPosition(FVector2D(400.0f, 200.0f));
        SaveSlot->SetSize(FVector2D(500.0f, 300.0f));
    }
    if (SaveLoadPanel)
    {
        SaveLoadPanel->SetVisibility(ESlateVisibility::Collapsed);
    }
}
