#include "Rules/CombatResolver.h"

void UCombatResolver::Initialize(UDiceRoller* InDiceRoller)
{
    DiceRoller = InDiceRoller;
}

EGCRollMode UCombatResolver::ResolveRollMode(bool bAdvantage, bool bDisadvantage) const
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

UDiceRoller* UCombatResolver::GetDiceRoller()
{
    if (!DiceRoller)
    {
        DiceRoller = NewObject<UDiceRoller>(GetTransientPackage());
    }
    return DiceRoller;
}

FAttackResult UCombatResolver::ResolveAttackRoll(const UGCCharacterSheet* Attacker, const FWeaponAttackData& Weapon, const UGCCharacterSheet* Defender, bool bAdvantage, bool bDisadvantage)
{
    FAttackResult Result;
    if (!Attacker || !Defender)
    {
        return Result;
    }

    const EGCRollMode RollMode = ResolveRollMode(bAdvantage, bDisadvantage);
    FDiceRollResult RollResult = GetDiceRoller()->RollDiceDetailed(1, 20, RollMode, TEXT("AttackRoll"));

    Result.Roll = RollResult.Total;
    Result.Modifier = Attacker->GetAbilityModifier(Weapon.AttackAbility) + (Weapon.bProficient ? Attacker->ProficiencyBonus : 0);
    Result.Total = Result.Roll + Result.Modifier;
    Result.TargetAC = Defender->ArmorClass;
    Result.bCriticalHit = (Result.Roll == 20);
    Result.bHit = Result.bCriticalHit || Result.Total >= Result.TargetAC;
    return Result;
}

int32 UCombatResolver::CalculateDamage(const FWeaponAttackData& Weapon, int32 Modifier, bool bCritical)
{
    const int32 DiceCount = bCritical ? Weapon.DamageDiceCount * 2 : Weapon.DamageDiceCount;
    const int32 DiceTotal = GetDiceRoller()->RollDice(FMath::Max(1, DiceCount), FMath::Max(2, Weapon.DamageDieSize));
    return FMath::Max(0, DiceTotal + Modifier);
}

int32 UCombatResolver::ApplyDamage(UGCCharacterSheet* Target, int32 Damage)
{
    if (!Target)
    {
        return 0;
    }

    Target->CurrentHP = FMath::Clamp(Target->CurrentHP - FMath::Max(0, Damage), 0, Target->MaxHP);
    return Target->CurrentHP;
}

TArray<UGCCharacterSheet*> UCombatResolver::ResolveInitiative(const TArray<UGCCharacterSheet*>& Combatants)
{
    struct FInitiativeRow
    {
        UGCCharacterSheet* Combatant = nullptr;
        int32 Initiative = 0;
    };

    TArray<FInitiativeRow> Rolls;
    for (UGCCharacterSheet* Combatant : Combatants)
    {
        if (!Combatant)
        {
            continue;
        }

        FInitiativeRow Row;
        Row.Combatant = Combatant;
        Row.Initiative = GetDiceRoller()->RollD20() + Combatant->GetAbilityModifier(EGCAbility::Dexterity);
        Rolls.Add(Row);
    }

    Rolls.Sort([](const FInitiativeRow& A, const FInitiativeRow& B)
    {
        return A.Initiative > B.Initiative;
    });

    TArray<UGCCharacterSheet*> Ordered;
    for (const FInitiativeRow& Row : Rolls)
    {
        Ordered.Add(Row.Combatant);
    }

    return Ordered;
}
