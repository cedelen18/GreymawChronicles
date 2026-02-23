#include "Quest/GCQuestSubsystem.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

DEFINE_LOG_CATEGORY_STATIC(LogQuestSubsystem, Log, All);

void UGCQuestSubsystem::StartQuest(const FString& QuestId, const FString& Title, const FString& Description)
{
    if (QuestLog.Contains(QuestId))
    {
        UE_LOG(LogQuestSubsystem, Warning, TEXT("Quest '%s' already exists, skipping start."), *QuestId);
        return;
    }

    FGCQuestEntry Entry;
    Entry.QuestId = QuestId;
    Entry.Title = Title;
    Entry.Description = Description;
    Entry.CurrentObjective = Description;
    Entry.Status = EGCQuestStatus::Active;

    QuestLog.Add(QuestId, Entry);
    OnQuestUpdated.Broadcast(QuestId, EGCQuestStatus::Active);

    UE_LOG(LogQuestSubsystem, Log, TEXT("Quest started: %s - %s"), *QuestId, *Title);
}

void UGCQuestSubsystem::UpdateObjective(const FString& QuestId, const FString& ObjectiveText)
{
    if (FGCQuestEntry* Entry = QuestLog.Find(QuestId))
    {
        Entry->CurrentObjective = ObjectiveText;
        UE_LOG(LogQuestSubsystem, Log, TEXT("Quest '%s' objective updated: %s"), *QuestId, *ObjectiveText);
    }
}

void UGCQuestSubsystem::CompleteQuest(const FString& QuestId)
{
    if (FGCQuestEntry* Entry = QuestLog.Find(QuestId))
    {
        Entry->Status = EGCQuestStatus::Completed;
        OnQuestUpdated.Broadcast(QuestId, EGCQuestStatus::Completed);
        UE_LOG(LogQuestSubsystem, Log, TEXT("Quest completed: %s"), *QuestId);
    }
}

void UGCQuestSubsystem::FailQuest(const FString& QuestId)
{
    if (FGCQuestEntry* Entry = QuestLog.Find(QuestId))
    {
        Entry->Status = EGCQuestStatus::Failed;
        OnQuestUpdated.Broadcast(QuestId, EGCQuestStatus::Failed);
        UE_LOG(LogQuestSubsystem, Log, TEXT("Quest failed: %s"), *QuestId);
    }
}

EGCQuestStatus UGCQuestSubsystem::GetQuestStatus(const FString& QuestId) const
{
    if (const FGCQuestEntry* Entry = QuestLog.Find(QuestId))
    {
        return Entry->Status;
    }
    return EGCQuestStatus::NotStarted;
}

TArray<FGCQuestEntry> UGCQuestSubsystem::GetActiveQuests() const
{
    TArray<FGCQuestEntry> Active;
    for (const auto& Pair : QuestLog)
    {
        if (Pair.Value.Status == EGCQuestStatus::Active)
        {
            Active.Add(Pair.Value);
        }
    }
    return Active;
}

TArray<FGCQuestEntry> UGCQuestSubsystem::GetAllQuests() const
{
    TArray<FGCQuestEntry> All;
    QuestLog.GenerateValueArray(All);
    return All;
}

const FGCQuestEntry* UGCQuestSubsystem::FindQuest(const FString& QuestId) const
{
    return QuestLog.Find(QuestId);
}

FString UGCQuestSubsystem::ToJSON() const
{
    TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
    TArray<TSharedPtr<FJsonValue>> QuestArray;

    for (const auto& Pair : QuestLog)
    {
        const FGCQuestEntry& Entry = Pair.Value;
        TSharedRef<FJsonObject> QuestObj = MakeShared<FJsonObject>();
        QuestObj->SetStringField(TEXT("quest_id"), Entry.QuestId);
        QuestObj->SetStringField(TEXT("title"), Entry.Title);
        QuestObj->SetStringField(TEXT("description"), Entry.Description);
        QuestObj->SetStringField(TEXT("objective"), Entry.CurrentObjective);
        QuestObj->SetNumberField(TEXT("status"), static_cast<int32>(Entry.Status));
        QuestArray.Add(MakeShared<FJsonValueObject>(QuestObj));
    }

    Root->SetArrayField(TEXT("quests"), QuestArray);

    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(Root, Writer);
    return OutputString;
}

bool UGCQuestSubsystem::FromJSON(const FString& Json)
{
    if (Json.IsEmpty())
    {
        QuestLog.Empty();
        return true;
    }

    TSharedPtr<FJsonObject> Root;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        UE_LOG(LogQuestSubsystem, Warning, TEXT("Failed to parse quest JSON."));
        return false;
    }

    QuestLog.Empty();

    const TArray<TSharedPtr<FJsonValue>>* QuestArray;
    if (Root->TryGetArrayField(TEXT("quests"), QuestArray))
    {
        for (const auto& Val : *QuestArray)
        {
            const TSharedPtr<FJsonObject>& QuestObj = Val->AsObject();
            if (!QuestObj.IsValid()) continue;

            FGCQuestEntry Entry;
            Entry.QuestId = QuestObj->GetStringField(TEXT("quest_id"));
            Entry.Title = QuestObj->GetStringField(TEXT("title"));
            Entry.Description = QuestObj->GetStringField(TEXT("description"));
            Entry.CurrentObjective = QuestObj->GetStringField(TEXT("objective"));
            Entry.Status = static_cast<EGCQuestStatus>(FMath::Clamp(
                static_cast<int32>(QuestObj->GetNumberField(TEXT("status"))), 0, 3));

            QuestLog.Add(Entry.QuestId, Entry);
        }
    }

    UE_LOG(LogQuestSubsystem, Log, TEXT("Loaded %d quests from JSON."), QuestLog.Num());
    return true;
}
