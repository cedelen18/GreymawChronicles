#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Quest/GCQuestSubsystem.h"
#include "GCQuestJournalWidget.generated.h"

class UVerticalBox;
class UTextBlock;

/**
 * Sprint L: Quest journal UI widget.
 * Displays a list of quests with title, objective, and status.
 * Toggleable visibility via ToggleVisibility().
 */
UCLASS()
class GREYMAWCHRONICLES_API UGCQuestJournalWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Toggle journal visibility on/off. */
    UFUNCTION(BlueprintCallable, Category = "UI|Quest")
    void ToggleVisibility();

    /** Refresh the quest list from the QuestSubsystem. */
    UFUNCTION(BlueprintCallable, Category = "UI|Quest")
    void RefreshQuestList();

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UVerticalBox> QuestListBox;

private:
    UFUNCTION()
    void HandleQuestUpdated(const FString& QuestId, EGCQuestStatus NewStatus);
};
