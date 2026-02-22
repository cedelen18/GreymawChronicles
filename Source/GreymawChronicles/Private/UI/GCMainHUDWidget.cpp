#include "UI/GCMainHUDWidget.h"

#include "DungeonMaster/DMBrainSubsystem.h"
#include "UI/DMTextPanelWidget.h"
#include "UI/GCHUDOverlayWidget.h"
#include "UI/GCPlayerInputWidget.h"

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

    if (UCanvasPanelSlot* OverlaySlot = Root->AddChildToCanvas(HUDOverlay))
    {
        OverlaySlot->SetAutoSize(true);
        OverlaySlot->SetPosition(FVector2D(20.0f, 20.0f));
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
}
