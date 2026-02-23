#include "Combat/GCCombatEncounter.h"
#include "Gameplay/GCCharacterSheet.h"
#include "Gameplay/DiceRoller.h"

DEFINE_LOG_CATEGORY_STATIC(LogCombatEncounter, Log, All);

const FString UGCCombatEncounter::PlayerTurnName = TEXT("Player");

void UGCCombatEncounter::InitializeEncounter(UGCCharacterSheet* Player, const TArray<FGCEnemyTemplate>& Enemies, UDiceRoller* InDiceRoller)
{
    PlayerSheetRef = Player;
    EnemyRoster = Enemies;
    DiceRollerRef = InDiceRoller;
    CurrentTurnIndex = 0;
    RoundNumber = 1;
    TurnOrder.Empty();

    UE_LOG(LogCombatEncounter, Log, TEXT("Combat initialized: %d enemies"), EnemyRoster.Num());
}

void UGCCombatEncounter::RollInitiative()
{
    if (!DiceRollerRef)
    {
        DiceRollerRef = NewObject<UDiceRoller>(this);
    }

    // Build initiative entries: (Roll + Modifier, Name)
    TArray<TPair<int32, FString>> InitiativeEntries;

    // Player initiative
    if (PlayerSheetRef)
    {
        const int32 DexMod = PlayerSheetRef->GetAbilityModifier(EGCAbility::Dexterity);
        const int32 Roll = DiceRollerRef->RollDice(1, 20);
        InitiativeEntries.Add(TPair<int32, FString>(Roll + DexMod, PlayerTurnName));
        UE_LOG(LogCombatEncounter, Log, TEXT("Player initiative: %d + %d = %d"), Roll, DexMod, Roll + DexMod);
    }

    // Enemy initiative
    for (int32 i = 0; i < EnemyRoster.Num(); ++i)
    {
        const int32 Roll = DiceRollerRef->RollDice(1, 20);
        const int32 Total = Roll + EnemyRoster[i].InitiativeModifier;
        const FString Name = FString::Printf(TEXT("%s_%d"), *EnemyRoster[i].CreatureName, i);
        InitiativeEntries.Add(TPair<int32, FString>(Total, Name));
        UE_LOG(LogCombatEncounter, Log, TEXT("%s initiative: %d + %d = %d"), *Name, Roll, EnemyRoster[i].InitiativeModifier, Total);
    }

    // Sort descending by initiative
    InitiativeEntries.Sort([](const TPair<int32, FString>& A, const TPair<int32, FString>& B)
    {
        return A.Key > B.Key;
    });

    TurnOrder.Empty();
    for (const auto& Entry : InitiativeEntries)
    {
        TurnOrder.Add(Entry.Value);
    }

    CurrentTurnIndex = 0;
    RoundNumber = 1;

    UE_LOG(LogCombatEncounter, Log, TEXT("Initiative order: %s"), *FString::Join(TurnOrder, TEXT(", ")));
}

bool UGCCombatEncounter::AdvanceTurn()
{
    if (IsCombatOver() || TurnOrder.Num() == 0)
    {
        return false;
    }

    CurrentTurnIndex++;
    if (CurrentTurnIndex >= TurnOrder.Num())
    {
        CurrentTurnIndex = 0;
        RoundNumber++;
        UE_LOG(LogCombatEncounter, Log, TEXT("=== Round %d ==="), RoundNumber);
    }

    return !IsCombatOver();
}

FString UGCCombatEncounter::GetCurrentTurnActor() const
{
    if (TurnOrder.Num() == 0 || CurrentTurnIndex < 0 || CurrentTurnIndex >= TurnOrder.Num())
    {
        return FString();
    }
    return TurnOrder[CurrentTurnIndex];
}

bool UGCCombatEncounter::IsPlayerTurn() const
{
    return GetCurrentTurnActor() == PlayerTurnName;
}

bool UGCCombatEncounter::IsCombatOver() const
{
    return GetOutcome() != ECombatOutcome::Ongoing;
}

ECombatOutcome UGCCombatEncounter::GetOutcome() const
{
    // Check defeat: player HP <= 0
    if (PlayerSheetRef && PlayerSheetRef->CurrentHP <= 0)
    {
        return ECombatOutcome::Defeat;
    }

    // Check victory: all enemies HP <= 0
    bool bAllEnemiesDead = true;
    for (const FGCEnemyTemplate& Enemy : EnemyRoster)
    {
        if (Enemy.CurrentHP > 0)
        {
            bAllEnemiesDead = false;
            break;
        }
    }

    if (EnemyRoster.Num() > 0 && bAllEnemiesDead)
    {
        return ECombatOutcome::Victory;
    }

    return ECombatOutcome::Ongoing;
}
