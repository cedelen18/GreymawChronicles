#include "Gameplay/GCCharacterSheet.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
static FString EnumToStringValue(const UEnum* EnumType, int64 Value)
{
    return EnumType ? EnumType->GetNameStringByValue(Value) : FString();
}

static bool TryStringToEnumValue(const UEnum* EnumType, const FString& Name, int64& OutValue)
{
    if (!EnumType)
    {
        return false;
    }

    OutValue = EnumType->GetValueByNameString(Name, EGetByNameFlags::None);
    return OutValue != INDEX_NONE;
}
}

int32 UGCCharacterSheet::GetAbilityModifier(EGCAbility Ability) const
{
    const int32* Score = AbilityScores.Find(Ability);
    const int32 SafeScore = Score ? *Score : 10;
    return FMath::FloorToInt((SafeScore - 10) / 2.0f);
}

EGCAbility UGCCharacterSheet::SkillToAbility(EGCSkill Skill)
{
    switch (Skill)
    {
    case EGCSkill::Acrobatics:
    case EGCSkill::SleightOfHand:
    case EGCSkill::Stealth:
        return EGCAbility::Dexterity;
    case EGCSkill::AnimalHandling:
    case EGCSkill::Insight:
    case EGCSkill::Medicine:
    case EGCSkill::Perception:
    case EGCSkill::Survival:
        return EGCAbility::Wisdom;
    case EGCSkill::Arcana:
    case EGCSkill::History:
    case EGCSkill::Investigation:
    case EGCSkill::Nature:
    case EGCSkill::Religion:
        return EGCAbility::Intelligence;
    case EGCSkill::Athletics:
        return EGCAbility::Strength;
    case EGCSkill::Deception:
    case EGCSkill::Intimidation:
    case EGCSkill::Performance:
    case EGCSkill::Persuasion:
        return EGCAbility::Charisma;
    default:
        return EGCAbility::Strength;
    }
}

int32 UGCCharacterSheet::GetSkillModifier(EGCSkill Skill) const
{
    const int32 Base = GetAbilityModifier(SkillToAbility(Skill));
    return Base + (SkillProficiencies.Contains(Skill) ? ProficiencyBonus : 0);
}

int32 UGCCharacterSheet::GetSavingThrowModifier(EGCAbility Ability) const
{
    const int32 Base = GetAbilityModifier(Ability);
    return Base + (SavingThrowProficiencies.Contains(Ability) ? ProficiencyBonus : 0);
}

FString UGCCharacterSheet::ToCompactJSON() const
{
    TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();

    Root->SetStringField(TEXT("n"), CharacterName);
    Root->SetStringField(TEXT("r"), EnumToStringValue(StaticEnum<EGCRace>(), static_cast<int64>(Race)));
    Root->SetStringField(TEXT("c"), EnumToStringValue(StaticEnum<EGCClass>(), static_cast<int64>(CharacterClass)));
    Root->SetNumberField(TEXT("l"), Level);
    Root->SetNumberField(TEXT("hp"), CurrentHP);
    Root->SetNumberField(TEXT("mhp"), MaxHP);
    Root->SetNumberField(TEXT("ac"), ArmorClass);
    Root->SetNumberField(TEXT("pb"), ProficiencyBonus);

    TSharedRef<FJsonObject> AbilitiesJson = MakeShared<FJsonObject>();
    for (const TPair<EGCAbility, int32>& Pair : AbilityScores)
    {
        AbilitiesJson->SetNumberField(EnumToStringValue(StaticEnum<EGCAbility>(), static_cast<int64>(Pair.Key)), Pair.Value);
    }
    Root->SetObjectField(TEXT("ab"), AbilitiesJson);

    TArray<TSharedPtr<FJsonValue>> SaveProfJson;
    for (const EGCAbility Ability : SavingThrowProficiencies)
    {
        SaveProfJson.Add(MakeShared<FJsonValueString>(EnumToStringValue(StaticEnum<EGCAbility>(), static_cast<int64>(Ability))));
    }
    Root->SetArrayField(TEXT("svp"), SaveProfJson);

    TArray<TSharedPtr<FJsonValue>> SkillProfJson;
    for (const EGCSkill Skill : SkillProficiencies)
    {
        SkillProfJson.Add(MakeShared<FJsonValueString>(EnumToStringValue(StaticEnum<EGCSkill>(), static_cast<int64>(Skill))));
    }
    Root->SetArrayField(TEXT("skp"), SkillProfJson);

    TArray<TSharedPtr<FJsonValue>> FeaturesJson;
    for (const FString& Feature : ClassFeatures)
    {
        FeaturesJson.Add(MakeShared<FJsonValueString>(Feature));
    }
    Root->SetArrayField(TEXT("f"), FeaturesJson);

    TArray<TSharedPtr<FJsonValue>> EquipmentJson;
    for (const FString& Item : Equipment)
    {
        EquipmentJson.Add(MakeShared<FJsonValueString>(Item));
    }
    Root->SetArrayField(TEXT("eq"), EquipmentJson);

    TSharedRef<FJsonObject> SpellSlotsJson = MakeShared<FJsonObject>();
    for (const TPair<int32, FGCSpellSlotState>& Pair : SpellSlotsByLevel)
    {
        TSharedRef<FJsonObject> SlotJson = MakeShared<FJsonObject>();
        SlotJson->SetNumberField(TEXT("m"), Pair.Value.Max);
        SlotJson->SetNumberField(TEXT("c"), Pair.Value.Current);
        SpellSlotsJson->SetObjectField(FString::FromInt(Pair.Key), SlotJson);
    }
    Root->SetObjectField(TEXT("ss"), SpellSlotsJson);

    TArray<TSharedPtr<FJsonValue>> SpellsJson;
    for (const FString& Spell : Spells)
    {
        SpellsJson.Add(MakeShared<FJsonValueString>(Spell));
    }
    Root->SetArrayField(TEXT("sp"), SpellsJson);

    TArray<TSharedPtr<FJsonValue>> CantripsJson;
    for (const FString& Cantrip : Cantrips)
    {
        CantripsJson.Add(MakeShared<FJsonValueString>(Cantrip));
    }
    Root->SetArrayField(TEXT("ct"), CantripsJson);

    FString Out;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Out);
    FJsonSerializer::Serialize(Root, Writer);
    return Out;
}

bool UGCCharacterSheet::FromJSON(const FString& InJSON, FString& OutError)
{
    OutError.Reset();

    TSharedPtr<FJsonObject> Root;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InJSON);
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        OutError = TEXT("Invalid JSON document");
        return false;
    }

    CharacterName = Root->GetStringField(TEXT("n"));

    int64 EnumValue = 0;
    if (TryStringToEnumValue(StaticEnum<EGCRace>(), Root->GetStringField(TEXT("r")), EnumValue))
    {
        Race = static_cast<EGCRace>(EnumValue);
    }

    if (TryStringToEnumValue(StaticEnum<EGCClass>(), Root->GetStringField(TEXT("c")), EnumValue))
    {
        CharacterClass = static_cast<EGCClass>(EnumValue);
    }

    Level = Root->GetIntegerField(TEXT("l"));
    CurrentHP = Root->GetIntegerField(TEXT("hp"));
    MaxHP = Root->GetIntegerField(TEXT("mhp"));
    ArmorClass = Root->GetIntegerField(TEXT("ac"));
    ProficiencyBonus = Root->GetIntegerField(TEXT("pb"));

    AbilityScores.Reset();
    const TSharedPtr<FJsonObject>* AbilitiesJson = nullptr;
    if (Root->TryGetObjectField(TEXT("ab"), AbilitiesJson) && AbilitiesJson && AbilitiesJson->IsValid())
    {
        for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : (*AbilitiesJson)->Values)
        {
            if (TryStringToEnumValue(StaticEnum<EGCAbility>(), Pair.Key, EnumValue))
            {
                AbilityScores.Add(static_cast<EGCAbility>(EnumValue), static_cast<int32>(Pair.Value->AsNumber()));
            }
        }
    }

    SavingThrowProficiencies.Reset();
    const TArray<TSharedPtr<FJsonValue>>* SaveProfJson = nullptr;
    if (Root->TryGetArrayField(TEXT("svp"), SaveProfJson) && SaveProfJson)
    {
        for (const TSharedPtr<FJsonValue>& Entry : *SaveProfJson)
        {
            if (TryStringToEnumValue(StaticEnum<EGCAbility>(), Entry->AsString(), EnumValue))
            {
                SavingThrowProficiencies.Add(static_cast<EGCAbility>(EnumValue));
            }
        }
    }

    SkillProficiencies.Reset();
    const TArray<TSharedPtr<FJsonValue>>* SkillProfJson = nullptr;
    if (Root->TryGetArrayField(TEXT("skp"), SkillProfJson) && SkillProfJson)
    {
        for (const TSharedPtr<FJsonValue>& Entry : *SkillProfJson)
        {
            if (TryStringToEnumValue(StaticEnum<EGCSkill>(), Entry->AsString(), EnumValue))
            {
                SkillProficiencies.Add(static_cast<EGCSkill>(EnumValue));
            }
        }
    }

    ClassFeatures.Reset();
    Root->TryGetStringArrayField(TEXT("f"), ClassFeatures);

    Equipment.Reset();
    Root->TryGetStringArrayField(TEXT("eq"), Equipment);

    SpellSlotsByLevel.Reset();
    const TSharedPtr<FJsonObject>* SpellSlotsJson = nullptr;
    if (Root->TryGetObjectField(TEXT("ss"), SpellSlotsJson) && SpellSlotsJson && SpellSlotsJson->IsValid())
    {
        for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : (*SpellSlotsJson)->Values)
        {
            const TSharedPtr<FJsonObject>* Slot = nullptr;
            if (!Pair.Value->TryGetObject(Slot) || !Slot || !Slot->IsValid())
            {
                continue;
            }

            FGCSpellSlotState SlotState;
            SlotState.Max = (*Slot)->GetIntegerField(TEXT("m"));
            SlotState.Current = (*Slot)->GetIntegerField(TEXT("c"));
            SpellSlotsByLevel.Add(FCString::Atoi(*Pair.Key), SlotState);
        }
    }

    Spells.Reset();
    Root->TryGetStringArrayField(TEXT("sp"), Spells);

    Cantrips.Reset();
    Root->TryGetStringArrayField(TEXT("ct"), Cantrips);

    return true;
}
