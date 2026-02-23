#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Combat/GCEnemyTemplate.h"
#include "GCCombatEncounter.generated.h"

class UGCCharacterSheet;
class UDiceRoller;
class UCombatResolver;

/**
 * Sprint L: Outcome of a combat encounter.
 */
UENUM(BlueprintType)
enum class ECombatOutcome : uint8
{
    Ongoing     UMETA(DisplayName = "Ongoing"),
    Victory     UMETA(DisplayName = "Victory"),
    Defeat      UMETA(DisplayName = "Defeat")
};

/**
 * Sprint L: Manages a turn-based combat encounter.
 * Tracks initiative order, turn cycling, and win/loss conditions.
 * Delegates actual attack/damage resolution to UCombatResolver.
 */
UCLASS(BlueprintType)
class GREYMAWCHRONICLES_API UGCCombatEncounter : public UObject
{
    GENERATED_BODY()

public:
    /** Set up the encounter with a player and a roster of enemies. */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitializeEncounter(UGCCharacterSheet* Player, const TArray<FGCEnemyTemplate>& Enemies, UDiceRoller* InDiceRoller);

    /** Roll initiative for all combatants and sort turn order. */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void RollInitiative();

    /** Advance to the next turn. Returns true if combat is still ongoing. */
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool AdvanceTurn();

    /** Get the name of the current turn's actor. */
    UFUNCTION(BlueprintPure, Category = "Combat")
    FString GetCurrentTurnActor() const;

    /** Is it currently the player's turn? */
    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsPlayerTurn() const;

    /** Is the combat finished? */
    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsCombatOver() const;

    /** Get the current combat outcome. */
    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombatOutcome GetOutcome() const;

    /** Get the current round number. */
    UFUNCTION(BlueprintPure, Category = "Combat")
    int32 GetRoundNumber() const { return RoundNumber; }

    /** Get the enemy roster (mutable for damage application). */
    TArray<FGCEnemyTemplate>& GetEnemyRoster() { return EnemyRoster; }
    const TArray<FGCEnemyTemplate>& GetEnemyRoster() const { return EnemyRoster; }

    /** Get the player sheet. */
    UGCCharacterSheet* GetPlayerSheet() const { return PlayerSheetRef; }

    /** The sorted turn order (actor names). */
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<FString> TurnOrder;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    int32 CurrentTurnIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    int32 RoundNumber = 1;

private:
    UPROPERTY()
    TArray<FGCEnemyTemplate> EnemyRoster;

    UPROPERTY()
    TObjectPtr<UGCCharacterSheet> PlayerSheetRef;

    UPROPERTY()
    TObjectPtr<UDiceRoller> DiceRollerRef;

    /** Player name constant used in TurnOrder. */
    static const FString PlayerTurnName;
};
