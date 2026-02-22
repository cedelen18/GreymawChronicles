#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Gameplay/AbilityCheckResolver.h"
#include "Gameplay/DiceRoller.h"
#include "Gameplay/GCCharacterSheet.h"
#include "Rules/CombatResolver.h"
#include "Rules/SpellSystem.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGCCharacterSheetModifierTest, "GreymawChronicles.Rules.CharacterSheetModifiers", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGCCharacterSheetJsonRoundTripTest, "GreymawChronicles.Rules.CharacterSheetJsonRoundTrip", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDiceRollerRangesTest, "GreymawChronicles.Rules.DiceRollerRanges", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAbilityCheckAdvantageCancelTest, "GreymawChronicles.Rules.AbilityCheckAdvantageCancel", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCombatResolverCoreMathTest, "GreymawChronicles.Rules.CombatResolver.CoreMath", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCritDoubleDiceOnlyTest, "GreymawChronicles.Rules.CombatResolver.CritDoubleDiceOnly", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpellSlotLogicTest, "GreymawChronicles.Rules.SpellSystem.SpellSlots", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGCCharacterSheetModifierTest::RunTest(const FString& Parameters)
{
    UGCCharacterSheet* Sheet = NewObject<UGCCharacterSheet>();
    Sheet->ProficiencyBonus = 2;
    Sheet->AbilityScores.Add(EGCAbility::Strength, 16);
    Sheet->AbilityScores.Add(EGCAbility::Dexterity, 8);
    Sheet->SavingThrowProficiencies.Add(EGCAbility::Strength);
    Sheet->SkillProficiencies.Add(EGCSkill::Athletics);

    TestEqual(TEXT("STR 16 should be +3"), Sheet->GetAbilityModifier(EGCAbility::Strength), 3);
    TestEqual(TEXT("DEX 8 should be -1"), Sheet->GetAbilityModifier(EGCAbility::Dexterity), -1);
    TestEqual(TEXT("Athletics should include proficiency"), Sheet->GetSkillModifier(EGCSkill::Athletics), 5);
    TestEqual(TEXT("STR save should include proficiency"), Sheet->GetSavingThrowModifier(EGCAbility::Strength), 5);
    return true;
}

bool FGCCharacterSheetJsonRoundTripTest::RunTest(const FString& Parameters)
{
    UGCCharacterSheet* Source = NewObject<UGCCharacterSheet>();
    Source->CharacterName = TEXT("Kael");
    Source->Race = EGCRace::Elf;
    Source->CharacterClass = EGCClass::Companion;
    Source->Level = 3;
    Source->MaxHP = 24;
    Source->CurrentHP = 19;
    Source->ArmorClass = 14;
    Source->ProficiencyBonus = 2;
    Source->AbilityScores.Add(EGCAbility::Dexterity, 16);
    Source->SavingThrowProficiencies.Add(EGCAbility::Dexterity);
    Source->SkillProficiencies.Add(EGCSkill::Stealth);
    Source->ClassFeatures = {TEXT("Sneak Attack")};
    Source->Equipment = {TEXT("Shortsword")};

    FGCSpellSlotState Level1;
    Level1.Max = 2;
    Level1.Current = 1;
    Source->SpellSlotsByLevel.Add(1, Level1);

    Source->Spells = {TEXT("Cure Wounds")};
    Source->Cantrips = {TEXT("Guidance")};

    const FString Json = Source->ToCompactJSON();

    UGCCharacterSheet* Target = NewObject<UGCCharacterSheet>();
    FString Error;
    const bool bLoaded = Target->FromJSON(Json, Error);

    TestTrue(TEXT("JSON should deserialize"), bLoaded);
    TestTrue(TEXT("Error should be empty"), Error.IsEmpty());
    TestEqual(TEXT("Name round-trip"), Target->CharacterName, Source->CharacterName);
    TestEqual(TEXT("Class round-trip"), static_cast<uint8>(Target->CharacterClass), static_cast<uint8>(Source->CharacterClass));
    TestEqual(TEXT("Stealth proficiency round-trip"), Target->SkillProficiencies.Contains(EGCSkill::Stealth), true);
    TestEqual(TEXT("Spell slot round-trip"), Target->SpellSlotsByLevel.FindRef(1).Current, 1);
    return true;
}

bool FDiceRollerRangesTest::RunTest(const FString& Parameters)
{
    UDiceRoller* Roller = NewObject<UDiceRoller>();

    for (int32 Index = 0; Index < 100; ++Index)
    {
        const int32 D20 = Roller->RollD20();
        TestTrue(TEXT("d20 must be 1..20"), D20 >= 1 && D20 <= 20);

        const FDiceRollResult ND6 = Roller->RollDiceDetailed(3, 6, EGCRollMode::Normal, TEXT("3d6"));
        TestTrue(TEXT("3d6 total in range"), ND6.Total >= 3 && ND6.Total <= 18);

        const FDiceRollResult Adv = Roller->RollDiceDetailed(1, 20, EGCRollMode::Advantage, TEXT("adv"));
        TestTrue(TEXT("Advantage chooses max"), Adv.Total == FMath::Max(Adv.PrimaryRoll, Adv.SecondaryRoll));

        const FDiceRollResult Dis = Roller->RollDiceDetailed(1, 20, EGCRollMode::Disadvantage, TEXT("dis"));
        TestTrue(TEXT("Disadvantage chooses min"), Dis.Total == FMath::Min(Dis.PrimaryRoll, Dis.SecondaryRoll));
    }

    return true;
}

bool FAbilityCheckAdvantageCancelTest::RunTest(const FString& Parameters)
{
    UGCCharacterSheet* Sheet = NewObject<UGCCharacterSheet>();
    Sheet->AbilityScores.Add(EGCAbility::Wisdom, 14);
    Sheet->ProficiencyBonus = 2;
    Sheet->SkillProficiencies.Add(EGCSkill::Perception);

    UAbilityCheckResolver* Resolver = NewObject<UAbilityCheckResolver>();
    Resolver->Initialize(NewObject<UDiceRoller>());

    const FAbilityCheckResult Result = Resolver->ResolveAbilityCheck(Sheet, EGCSkill::Perception, 12, true, true);
    TestEqual(TEXT("Advantage and disadvantage should cancel to normal"), static_cast<uint8>(Result.RollMode), static_cast<uint8>(EGCRollMode::Normal));
    TestEqual(TEXT("Perception modifier should be +4"), Result.Modifier, 4);
    TestTrue(TEXT("Roll should still be d20 range"), Result.Roll >= 1 && Result.Roll <= 20);
    return true;
}

bool FCombatResolverCoreMathTest::RunTest(const FString& Parameters)
{
    UCombatResolver* Resolver = NewObject<UCombatResolver>();
    Resolver->Initialize(NewObject<UDiceRoller>());

    UGCCharacterSheet* Attacker = NewObject<UGCCharacterSheet>();
    Attacker->AbilityScores.Add(EGCAbility::Strength, 16);
    Attacker->ProficiencyBonus = 2;

    UGCCharacterSheet* Defender = NewObject<UGCCharacterSheet>();
    Defender->ArmorClass = 12;

    FWeaponAttackData Weapon;
    Weapon.AttackAbility = EGCAbility::Strength;
    Weapon.bProficient = true;

    const FAttackResult Result = Resolver->ResolveAttackRoll(Attacker, Weapon, Defender, false, false);
    TestEqual(TEXT("Attack modifier is ability + proficiency"), Result.Modifier, 5);
    TestEqual(TEXT("Total equals roll + modifier"), Result.Total, Result.Roll + Result.Modifier);

    return true;
}

bool FCritDoubleDiceOnlyTest::RunTest(const FString& Parameters)
{
    UCombatResolver* Resolver = NewObject<UCombatResolver>();
    Resolver->Initialize(NewObject<UDiceRoller>());

    FWeaponAttackData Weapon;
    Weapon.DamageDiceCount = 1;
    Weapon.DamageDieSize = 1;

    const int32 NonCrit = Resolver->CalculateDamage(Weapon, 4, false);
    const int32 Crit = Resolver->CalculateDamage(Weapon, 4, true);

    TestEqual(TEXT("Non-crit uses one die + mod"), NonCrit, 5);
    TestEqual(TEXT("Crit doubles dice only (2d1 + mod)"), Crit, 6);
    return true;
}

bool FSpellSlotLogicTest::RunTest(const FString& Parameters)
{
    USpellSystem* SpellSystem = NewObject<USpellSystem>();
    SpellSystem->Initialize(NewObject<UDiceRoller>());

    UGCCharacterSheet* Caster = NewObject<UGCCharacterSheet>();
    FGCSpellSlotState Slots;
    Slots.Max = 2;
    Slots.Current = 2;
    Caster->SpellSlotsByLevel.Add(1, Slots);

    TestTrue(TEXT("Has level 1 slot initially"), SpellSystem->HasSpellSlot(Caster, 1));
    TestTrue(TEXT("Cantrip always available"), SpellSystem->HasSpellSlot(Caster, 0));
    TestTrue(TEXT("First expenditure succeeds"), SpellSystem->ExpendSpellSlot(Caster, 1));
    TestTrue(TEXT("Second expenditure succeeds"), SpellSystem->ExpendSpellSlot(Caster, 1));
    TestFalse(TEXT("Third expenditure fails"), SpellSystem->ExpendSpellSlot(Caster, 1));
    return true;
}

#endif
