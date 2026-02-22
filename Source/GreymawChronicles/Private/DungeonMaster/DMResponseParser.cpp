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

    auto ParseActions = [this, &Out](const TArray<TSharedPtr<FJsonValue>>* ActionsJson, TArray<FDMAction>& OutActions)
    {
        if (!ActionsJson)
        {
            return;
        }

        for (const TSharedPtr<FJsonValue>& ActionValue : *ActionsJson)
        {
            const TSharedPtr<FJsonObject>* ActionObj = nullptr;
            if (!ActionValue.IsValid() || !ActionValue->TryGetObject(ActionObj) || !ActionObj || !ActionObj->IsValid())
            {
                continue;
            }

            FDMAction ParsedAction;
            (*ActionObj)->TryGetStringField(TEXT("action"), ParsedAction.Action);
            (*ActionObj)->TryGetStringField(TEXT("animation"), ParsedAction.Animation);
            (*ActionObj)->TryGetStringField(TEXT("actor"), ParsedAction.Actor);
            (*ActionObj)->TryGetStringField(TEXT("target"), ParsedAction.Target);

            double Delay = 0.0;
            if ((*ActionObj)->TryGetNumberField(TEXT("delay"), Delay) || (*ActionObj)->TryGetNumberField(TEXT("delay_seconds"), Delay))
            {
                ParsedAction.DelaySeconds = FMath::Max(0.0, static_cast<float>(Delay));
            }

            double MoveSpeed = 0.0;
            if ((*ActionObj)->TryGetNumberField(TEXT("move_speed"), MoveSpeed) || (*ActionObj)->TryGetNumberField(TEXT("speed"), MoveSpeed))
            {
                ParsedAction.MoveSpeedUnitsPerSecond = FMath::Max(50.0f, static_cast<float>(MoveSpeed));
            }

            const TSharedPtr<FJsonObject>* TargetLocationObj = nullptr;
            if ((*ActionObj)->TryGetObjectField(TEXT("target_location"), TargetLocationObj) && TargetLocationObj && TargetLocationObj->IsValid())
            {
                double X = 0.0, Y = 0.0, Z = 0.0;
                const bool bHasX = (*TargetLocationObj)->TryGetNumberField(TEXT("x"), X);
                const bool bHasY = (*TargetLocationObj)->TryGetNumberField(TEXT("y"), Y);
                const bool bHasZ = (*TargetLocationObj)->TryGetNumberField(TEXT("z"), Z);
                if (bHasX && bHasY)
                {
                    ParsedAction.bHasMoveTarget = true;
                    ParsedAction.MoveTarget = FVector(static_cast<float>(X), static_cast<float>(Y), bHasZ ? static_cast<float>(Z) : 0.0f);
                }
            }

            if (!ParsedAction.Action.IsEmpty() && !IsAllowedAnimationAction(ParsedAction.Action) &&
                !ParsedAction.Action.Equals(TEXT("move"), ESearchCase::IgnoreCase) &&
                !ParsedAction.Action.Equals(TEXT("wait"), ESearchCase::IgnoreCase))
            {
                Out.Error = FString::Printf(TEXT("Unknown or disallowed action: %s"), *ParsedAction.Action);
            }

            OutActions.Add(ParsedAction);
        }
    };

    auto ParseWorldChanges = [](const TArray<TSharedPtr<FJsonValue>>* ChangesJson, TArray<FDMWorldChange>& OutChanges)
    {
        if (!ChangesJson)
        {
            return;
        }

        for (const TSharedPtr<FJsonValue>& ChangeValue : *ChangesJson)
        {
            const TSharedPtr<FJsonObject>* ChangeObj = nullptr;
            if (!ChangeValue.IsValid() || !ChangeValue->TryGetObject(ChangeObj) || !ChangeObj || !ChangeObj->IsValid())
            {
                continue;
            }

            FDMWorldChange Change;
            (*ChangeObj)->TryGetStringField(TEXT("type"), Change.Type);
            (*ChangeObj)->TryGetStringField(TEXT("target"), Change.Key);
            (*ChangeObj)->TryGetStringField(TEXT("key"), Change.Key);

            double Amount = 0.0;
            if ((*ChangeObj)->TryGetNumberField(TEXT("amount"), Amount) || (*ChangeObj)->TryGetNumberField(TEXT("magnitude"), Amount))
            {
                Change.Value = FString::FromInt(static_cast<int32>(Amount));
            }
            else
            {
                (*ChangeObj)->TryGetStringField(TEXT("value"), Change.Value);
            }

            OutChanges.Add(Change);
        }
    };

    const TArray<TSharedPtr<FJsonValue>>* ActionsJson = nullptr;
    if (Root->TryGetArrayField(TEXT("actions"), ActionsJson))
    {
        ParseActions(ActionsJson, Out.Actions);
    }

    const TSharedPtr<FJsonObject>* CheckObj = nullptr;
    if (Root->TryGetObjectField(TEXT("check_required"), CheckObj) && CheckObj && CheckObj->IsValid())
    {
        Out.Check.bCheckRequired = true;
        (*CheckObj)->TryGetStringField(TEXT("type"), Out.Check.CheckType);
        double DCValue = 0.0;
        if ((*CheckObj)->TryGetNumberField(TEXT("dc"), DCValue))
        {
            Out.Check.DC = static_cast<int32>(DCValue);
        }
    }

    const TSharedPtr<FJsonObject>* SuccessObj = nullptr;
    if (Root->TryGetObjectField(TEXT("on_success"), SuccessObj) && SuccessObj && SuccessObj->IsValid())
    {
        (*SuccessObj)->TryGetStringField(TEXT("narration"), Out.SuccessBranch.Narration);

        const TArray<TSharedPtr<FJsonValue>>* SuccessActions = nullptr;
        if ((*SuccessObj)->TryGetArrayField(TEXT("actions"), SuccessActions))
        {
            ParseActions(SuccessActions, Out.SuccessBranch.Actions);
        }

        const TArray<TSharedPtr<FJsonValue>>* SuccessChanges = nullptr;
        if ((*SuccessObj)->TryGetArrayField(TEXT("world_changes"), SuccessChanges))
        {
            ParseWorldChanges(SuccessChanges, Out.WorldChanges);
        }
    }

    const TSharedPtr<FJsonObject>* FailureObj = nullptr;
    if (Root->TryGetObjectField(TEXT("on_failure"), FailureObj) && FailureObj && FailureObj->IsValid())
    {
        (*FailureObj)->TryGetStringField(TEXT("narration"), Out.FailureBranch.Narration);

        const TArray<TSharedPtr<FJsonValue>>* FailureActions = nullptr;
        if ((*FailureObj)->TryGetArrayField(TEXT("actions"), FailureActions))
        {
            ParseActions(FailureActions, Out.FailureBranch.Actions);
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
