#include "Rules/SpellSystem.h"

void USpellSystem::Initialize(UDiceRoller* InDiceRoller)
{
    DiceRoller = InDiceRoller;
    if (Definitions.Num() == 0)
    {
        BuildMVPDefinitions();
    }
}

void USpellSystem::BuildMVPDefinitions()
{
    auto Add = [this](const FString& Id, int32 Level, bool bAttack)
    {
        FSpellDefinition Def;
        Def.SpellId = Id;
        Def.SpellLevel = Level;
        Def.bRequiresAttackRoll = bAttack;
        Definitions.Add(Id, Def);
    };

    Add(TEXT("fire_bolt"), 0, true);
    Add(TEXT("sacred_flame"), 0, false);
    Add(TEXT("guidance"), 0, false);
    Add(TEXT("spare_the_dying"), 0, false);
    Add(TEXT("mage_hand"), 0, false);
    Add(TEXT("prestidigitation"), 0, false);
    Add(TEXT("magic_missile"), 1, false);
    Add(TEXT("shield"), 1, false);
    Add(TEXT("thunderwave"), 1, false);
    Add(TEXT("cure_wounds"), 1, false);
    Add(TEXT("healing_word"), 1, false);
    Add(TEXT("guiding_bolt"), 1, true);
    Add(TEXT("scorching_ray"), 2, true);
    Add(TEXT("misty_step"), 2, false);
    Add(TEXT("spiritual_weapon"), 2, false);
    Add(TEXT("aid"), 2, false);
}

bool USpellSystem::HasSpellSlot(const UGCCharacterSheet* Caster, int32 SpellLevel) const
{
    if (Definitions.Num() == 0)
    {
        const_cast<USpellSystem*>(this)->BuildMVPDefinitions();
    }

    if (!Caster)
    {
        return false;
    }

    if (SpellLevel <= 0)
    {
        return true;
    }

    if (const FGCSpellSlotState* SlotState = Caster->SpellSlotsByLevel.Find(SpellLevel))
    {
        return SlotState->Current > 0;
    }

    return false;
}

bool USpellSystem::ExpendSpellSlot(UGCCharacterSheet* Caster, int32 SpellLevel)
{
    if (!Caster)
    {
        return false;
    }

    if (SpellLevel <= 0)
    {
        return true;
    }

    FGCSpellSlotState* SlotState = Caster->SpellSlotsByLevel.Find(SpellLevel);
    if (!SlotState || SlotState->Current <= 0)
    {
        return false;
    }

    --SlotState->Current;
    return true;
}

EGCAbility USpellSystem::ResolveSpellcastingAbility(const UGCCharacterSheet* Caster) const
{
    if (!Caster)
    {
        return EGCAbility::Intelligence;
    }

    switch (Caster->CharacterClass)
    {
    case EGCClass::Cleric:
        return EGCAbility::Wisdom;
    case EGCClass::Wizard:
        return EGCAbility::Intelligence;
    default:
        return EGCAbility::Charisma;
    }
}

int32 USpellSystem::GetSpellSaveDC(const UGCCharacterSheet* Caster) const
{
    if (!Caster)
    {
        return 8;
    }

    return 8 + Caster->ProficiencyBonus + Caster->GetAbilityModifier(ResolveSpellcastingAbility(Caster));
}

int32 USpellSystem::GetSpellAttackModifier(const UGCCharacterSheet* Caster) const
{
    if (!Caster)
    {
        return 0;
    }

    return Caster->ProficiencyBonus + Caster->GetAbilityModifier(ResolveSpellcastingAbility(Caster));
}

EGCRollMode USpellSystem::ResolveRollMode(bool bAdvantage, bool bDisadvantage) const
{
    if (bAdvantage && bDisadvantage)
    {
        return EGCRollMode::Normal;
    }
    if (bAdvantage)
    {
        return EGCRollMode::Advantage;
    }
    if (bDisadvantage)
    {
        return EGCRollMode::Disadvantage;
    }
    return EGCRollMode::Normal;
}

UDiceRoller* USpellSystem::GetDiceRoller()
{
    if (!DiceRoller)
    {
        DiceRoller = NewObject<UDiceRoller>(GetTransientPackage());
    }
    return DiceRoller;
}

FSpellAttackResult USpellSystem::ResolveSpellAttack(const UGCCharacterSheet* Caster, int32 TargetArmorClass, bool bAdvantage, bool bDisadvantage)
{
    FSpellAttackResult Result;
    if (!Caster)
    {
        return Result;
    }

    const FDiceRollResult Roll = GetDiceRoller()->RollDiceDetailed(1, 20, ResolveRollMode(bAdvantage, bDisadvantage), TEXT("SpellAttack"));
    Result.Roll = Roll.Total;
    Result.Modifier = GetSpellAttackModifier(Caster);
    Result.Total = Result.Roll + Result.Modifier;
    Result.bCriticalHit = (Result.Roll == 20);
    Result.bHit = Result.bCriticalHit || Result.Total >= TargetArmorClass;
    return Result;
}

bool USpellSystem::TryGetSpellDefinition(const FString& SpellId, FSpellDefinition& OutSpell) const
{
    if (const FSpellDefinition* Found = Definitions.Find(SpellId))
    {
        OutSpell = *Found;
        return true;
    }

    OutSpell = FSpellDefinition();
    return false;
}
