#include "DungeonMaster/DMResponseParser.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

FDMResponse UDMResponseParser::ParseResponse(const FString& RawText) const
{
    FDMResponse Out;

    const FString CandidateJson = RepairJson(RawText);
    TSharedPtr<FJsonObject> Root;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(CandidateJson);

    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        Out.Error = TEXT("Failed to parse DM JSON response after repair.");
        return Out;
    }

    Root->TryGetStringField(TEXT("narration"), Out.Narration);
    Root->TryGetStringField(TEXT("companion_reaction"), Out.CompanionReaction);

    const TArray<TSharedPtr<FJsonValue>>* ActionsJson = nullptr;
    if (Root->TryGetArrayField(TEXT("actions"), ActionsJson))
    {
        for (const TSharedPtr<FJsonValue>& ActionValue : *ActionsJson)
        {
            const TSharedPtr<FJsonObject>* ActionObj = nullptr;
            if (!ActionValue.IsValid() || !ActionValue->TryGetObject(ActionObj) || !ActionObj || !ActionObj->IsValid())
            {
                continue;
            }

            FDMAction ParsedAction;
            (*ActionObj)->TryGetStringField(TEXT("action"), ParsedAction.Action);
            (*ActionObj)->TryGetStringField(TEXT("actor"), ParsedAction.Actor);
            (*ActionObj)->TryGetStringField(TEXT("target"), ParsedAction.Target);

            if (!IsAllowedAnimationAction(ParsedAction.Action))
            {
                Out.Error = FString::Printf(TEXT("Unknown or disallowed action: %s"), *ParsedAction.Action);
            }

            Out.Actions.Add(ParsedAction);
        }
    }

    Out.bValid = Out.Error.IsEmpty();
    return Out;
}

FString UDMResponseParser::RepairJson(const FString& InJson) const
{
    FString Repaired = InJson;
    Repaired.ReplaceInline(TEXT(",}"), TEXT("}"));
    Repaired.ReplaceInline(TEXT(",]"), TEXT("]"));

    const int32 FirstBrace = Repaired.Find(TEXT("{"));
    const int32 LastBrace = Repaired.Find(TEXT("}"), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
    if (FirstBrace != INDEX_NONE && LastBrace != INDEX_NONE && LastBrace > FirstBrace)
    {
        Repaired = Repaired.Mid(FirstBrace, LastBrace - FirstBrace + 1);
    }

    return Repaired;
}

bool UDMResponseParser::IsAllowedAnimationAction(const FString& ActionName) const
{
    static const TSet<FString> AllowedActions = {
        TEXT("idle"), TEXT("walk"), TEXT("run"), TEXT("kneel"), TEXT("sit"), TEXT("stand_up"),
        TEXT("attack_melee"), TEXT("attack_ranged"), TEXT("cast_spell"), TEXT("block"), TEXT("dodge"),
        TEXT("take_hit"), TEXT("die"), TEXT("talk_gesture"), TEXT("point"), TEXT("nod"), TEXT("shake_head")
    };

    return AllowedActions.Contains(ActionName);
}
