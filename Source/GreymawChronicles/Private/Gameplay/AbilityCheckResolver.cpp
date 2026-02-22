#include "Gameplay/AbilityCheckResolver.h"

void UAbilityCheckResolver::Initialize(UDiceRoller* InDiceRoller)
{
    DiceRoller = InDiceRoller;
}

EGCRollMode UAbilityCheckResolver::ResolveRollMode(bool bAdvantage, bool bDisadvantage) const
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

int32 UAbilityCheckResolver::RollD20ForMode(EGCRollMode Mode) const
{
    UDiceRoller* LocalRoller = DiceRoller;
    if (!LocalRoller)
    {
        LocalRoller = NewObject<UDiceRoller>(GetTransientPackage());
    }

    switch (Mode)
    {
    case EGCRollMode::Advantage:
        return LocalRoller->RollD20WithAdvantage();
    case EGCRollMode::Disadvantage:
        return LocalRoller->RollD20WithDisadvantage();
    default:
        return LocalRoller->RollD20();
    }
}

FAbilityCheckResult UAbilityCheckResolver::ResolveAbilityCheck(const UGCCharacterSheet* Sheet, EGCSkill Skill, int32 DC, bool bAdvantage, bool bDisadvantage)
{
    FAbilityCheckResult Result;
    Result.DC = DC;
    Result.RollMode = ResolveRollMode(bAdvantage, bDisadvantage);
    Result.CheckType = FString::Printf(TEXT("Skill:%s"), *StaticEnum<EGCSkill>()->GetNameStringByValue(static_cast<int64>(Skill)));

    if (!Sheet)
    {
        return Result;
    }

    Result.Modifier = Sheet->GetSkillModifier(Skill);
    Result.Roll = RollD20ForMode(Result.RollMode);
    Result.Total = Result.Roll + Result.Modifier;
    Result.bCriticalSuccess = (Result.Roll == 20);
    Result.bCriticalFailure = (Result.Roll == 1);
    Result.bSuccess = (Result.Total >= DC);
    return Result;
}

FAbilityCheckResult UAbilityCheckResolver::ResolveSavingThrow(const UGCCharacterSheet* Sheet, EGCAbility Ability, int32 DC, bool bAdvantage, bool bDisadvantage)
{
    FAbilityCheckResult Result;
    Result.DC = DC;
    Result.RollMode = ResolveRollMode(bAdvantage, bDisadvantage);
    Result.CheckType = FString::Printf(TEXT("Save:%s"), *StaticEnum<EGCAbility>()->GetNameStringByValue(static_cast<int64>(Ability)));

    if (!Sheet)
    {
        return Result;
    }

    Result.Modifier = Sheet->GetSavingThrowModifier(Ability);
    Result.Roll = RollD20ForMode(Result.RollMode);
    Result.Total = Result.Roll + Result.Modifier;
    Result.bCriticalSuccess = (Result.Roll == 20);
    Result.bCriticalFailure = (Result.Roll == 1);
    Result.bSuccess = (Result.Total >= DC);
    return Result;
}
