#include "DungeonMaster/DMWorldStateSubsystem.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

void UDMWorldStateSubsystem::SetState(const FString& Category, const FString& Key, const FString& Value)
{
    TMap<FString, FString>& Inner = StateStore.FindOrAdd(Category);
    Inner.FindOrAdd(Key) = Value;
}

FString UDMWorldStateSubsystem::GetState(const FString& Category, const FString& Key) const
{
    if (const TMap<FString, FString>* Inner = StateStore.Find(Category))
    {
        if (const FString* Found = Inner->Find(Key))
        {
            return *Found;
        }
    }
    return FString();
}

bool UDMWorldStateSubsystem::HasState(const FString& Category, const FString& Key) const
{
    if (const TMap<FString, FString>* Inner = StateStore.Find(Category))
    {
        return Inner->Contains(Key);
    }
    return false;
}

bool UDMWorldStateSubsystem::ClearState(const FString& Category, const FString& Key)
{
    if (TMap<FString, FString>* Inner = StateStore.Find(Category))
    {
        return Inner->Remove(Key) > 0;
    }
    return false;
}

FString UDMWorldStateSubsystem::ToJSON() const
{
    TSharedRef<FJsonObject> RootObj = MakeShared<FJsonObject>();

    for (const auto& CategoryPair : StateStore)
    {
        TSharedRef<FJsonObject> CategoryObj = MakeShared<FJsonObject>();
        for (const auto& KVPair : CategoryPair.Value)
        {
            CategoryObj->SetStringField(KVPair.Key, KVPair.Value);
        }
        RootObj->SetObjectField(CategoryPair.Key, CategoryObj);
    }

    FString Output;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
    FJsonSerializer::Serialize(RootObj, Writer);
    return Output;
}

bool UDMWorldStateSubsystem::FromJSON(const FString& JsonText)
{
    ClearAll();

    if (JsonText.IsEmpty())
    {
        return true;
    }

    TSharedPtr<FJsonObject> RootObj;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
    if (!FJsonSerializer::Deserialize(Reader, RootObj) || !RootObj.IsValid())
    {
        return false;
    }

    for (const auto& CategoryPair : RootObj->Values)
    {
        const TSharedPtr<FJsonObject>* CategoryObj = nullptr;
        if (CategoryPair.Value->TryGetObject(CategoryObj) && CategoryObj && CategoryObj->IsValid())
        {
            for (const auto& KVPair : (*CategoryObj)->Values)
            {
                FString Value;
                if (KVPair.Value->TryGetString(Value))
                {
                    SetState(CategoryPair.Key, KVPair.Key, Value);
                }
            }
        }
    }

    return true;
}

void UDMWorldStateSubsystem::ClearAll()
{
    StateStore.Empty();
}
