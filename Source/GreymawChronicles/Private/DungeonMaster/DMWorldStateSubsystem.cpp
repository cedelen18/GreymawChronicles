#include "DungeonMaster/DMWorldStateSubsystem.h"

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
