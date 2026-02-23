#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GCMainHUDWidget.generated.h"

class UDMTextPanelWidget;
class UGCPlayerInputWidget;
class UGCHUDOverlayWidget;
class UGCDebugOverlayWidget;
class UGCActionFeedbackWidget;
class UGCQuestJournalWidget;
class UGCSaveLoadWidget;
class UDMBrainSubsystem;

UCLASS()
class GREYMAWCHRONICLES_API UGCMainHUDWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UDMTextPanelWidget> DMTextPanel;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UGCPlayerInputWidget> PlayerInput;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UGCHUDOverlayWidget> HUDOverlay;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UGCDebugOverlayWidget> DebugOverlay;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UGCActionFeedbackWidget> ActionFeedback;

    /** Sprint L: Quest journal panel. */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UGCQuestJournalWidget> QuestJournal;

    /** Sprint L: Save/load panel. */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UGCSaveLoadWidget> SaveLoadPanel;

private:
    UFUNCTION()
    void HandleNarration(const FString& NarrationText);

    void EnsureFallbackWidgets();
    TWeakObjectPtr<UDMBrainSubsystem> DMBrain;
};
