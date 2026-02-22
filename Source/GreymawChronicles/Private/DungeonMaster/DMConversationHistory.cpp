#include "DungeonMaster/DMConversationHistory.h"

#include "Serialization/JsonWriter.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UDMConversationHistory::AddExchange(const FDMExchange& Exchange)
{
    Exchanges.Add(Exchange);
    while (Exchanges.Num() > Capacity)
    {
        Exchanges.RemoveAt(0);
    }
}

TArray<FDMExchange> UDMConversationHistory::GetRecentExchanges() const
{
    return Exchanges;
}

FString UDMConversationHistory::ToPromptString() const
{
    FString Out;
    for (const FDMExchange& Entry : Exchanges)
    {
        Out += FString::Printf(TEXT("[%s][%s]\nPlayer: %s\nDM: %s\n"),
            *Entry.TimestampUtc.ToString(),
            *Entry.SceneID,
            *Entry.PlayerInput,
            *Entry.DMResponseSummary);
    }
    return Out;
}

void UDMConversationHistory::Clear()
{
    Exchanges.Reset();
}

FString UDMConversationHistory::SerializeToJSON() const
{
    TArray<TSharedPtr<FJsonValue>> JsonExchanges;
    for (const FDMExchange& Entry : Exchanges)
    {
        TSharedRef<FJsonObject> Obj = MakeShared<FJsonObject>();
        Obj->SetStringField(TEXT("player_input"), Entry.PlayerInput);
        Obj->SetStringField(TEXT("dm_response_summary"), Entry.DMResponseSummary);
        Obj->SetStringField(TEXT("scene_id"), Entry.SceneID);
        Obj->SetStringField(TEXT("timestamp_utc"), Entry.TimestampUtc.ToIso8601());
        JsonExchanges.Add(MakeShared<FJsonValueObject>(Obj));
    }

    TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
    Root->SetArrayField(TEXT("exchanges"), JsonExchanges);

    FString Json;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Json);
    FJsonSerializer::Serialize(Root, Writer);
    return Json;
}

bool UDMConversationHistory::DeserializeFromJSON(const FString& JsonText)
{
    TSharedPtr<FJsonObject> Root;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        return false;
    }

    const TArray<TSharedPtr<FJsonValue>>* JsonExchanges = nullptr;
    if (!Root->TryGetArrayField(TEXT("exchanges"), JsonExchanges))
    {
        return false;
    }

    Exchanges.Reset();
    for (const TSharedPtr<FJsonValue>& JsonValue : *JsonExchanges)
    {
        const TSharedPtr<FJsonObject>* Obj = nullptr;
        if (!JsonValue.IsValid() || !JsonValue->TryGetObject(Obj) || !Obj || !Obj->IsValid())
        {
            continue;
        }

        FDMExchange Entry;
        (*Obj)->TryGetStringField(TEXT("player_input"), Entry.PlayerInput);
        (*Obj)->TryGetStringField(TEXT("dm_response_summary"), Entry.DMResponseSummary);
        (*Obj)->TryGetStringField(TEXT("scene_id"), Entry.SceneID);

        FString TimestampIso;
        if ((*Obj)->TryGetStringField(TEXT("timestamp_utc"), TimestampIso))
        {
            FDateTime::ParseIso8601(*TimestampIso, Entry.TimestampUtc);
        }

        AddExchange(Entry);
    }

    return true;
}
