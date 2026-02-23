#include "UI/GCQuestJournalWidget.h"
#include "Quest/GCQuestSubsystem.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuestJournal, Log, All);

void UGCQuestJournalWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind to quest updates
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UGCQuestSubsystem* QuestSub = GI->GetSubsystem<UGCQuestSubsystem>())
        {
            QuestSub->OnQuestUpdated.AddDynamic(this, &UGCQuestJournalWidget::HandleQuestUpdated);
        }
    }

    // Start hidden
    SetVisibility(ESlateVisibility::Collapsed);
}

void UGCQuestJournalWidget::ToggleVisibility()
{
    if (GetVisibility() == ESlateVisibility::Visible)
    {
        SetVisibility(ESlateVisibility::Collapsed);
    }
    else
    {
        RefreshQuestList();
        SetVisibility(ESlateVisibility::Visible);
    }
}

void UGCQuestJournalWidget::RefreshQuestList()
{
    if (!QuestListBox)
    {
        UE_LOG(LogQuestJournal, Warning, TEXT("QuestListBox not bound, cannot refresh."));
        return;
    }

    QuestListBox->ClearChildren();

    UGameInstance* GI = GetGameInstance();
    if (!GI) return;

    UGCQuestSubsystem* QuestSub = GI->GetSubsystem<UGCQuestSubsystem>();
    if (!QuestSub) return;

    const TArray<FGCQuestEntry> AllQuests = QuestSub->GetAllQuests();

    for (const FGCQuestEntry& Quest : AllQuests)
    {
        UTextBlock* QuestText = NewObject<UTextBlock>(this);
        if (!QuestText) continue;

        FString StatusStr;
        switch (Quest.Status)
        {
        case EGCQuestStatus::Active:    StatusStr = TEXT("[Active]"); break;
        case EGCQuestStatus::Completed: StatusStr = TEXT("[Complete]"); break;
        case EGCQuestStatus::Failed:    StatusStr = TEXT("[Failed]"); break;
        default:                        StatusStr = TEXT("[Unknown]"); break;
        }

        const FString DisplayText = FString::Printf(TEXT("%s %s\n  %s"), *StatusStr, *Quest.Title, *Quest.CurrentObjective);
        QuestText->SetText(FText::FromString(DisplayText));

        QuestListBox->AddChild(QuestText);
    }
}

void UGCQuestJournalWidget::HandleQuestUpdated(const FString& QuestId, EGCQuestStatus NewStatus)
{
    // Auto-refresh if visible
    if (GetVisibility() == ESlateVisibility::Visible)
    {
        RefreshQuestList();
    }
}
