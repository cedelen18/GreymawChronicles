#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DungeonMaster/DMTypes.h"
#include "Gameplay/AbilityCheckResolver.h"
#include "DMBrainSubsystem.generated.h"

class UOllamaSubsystem;
class UDMPromptBuilder;
class UDMResponseParser;
class UDMConversationHistory;
class UDMIntentClassifier;
class UDMNarrationPool;
class UDMWorldStateSubsystem;
class UCombatResolver;
class USpellSystem;
class UDiceRoller;
class UGCCharacterSheet;
struct FDMIntentResult;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDMNarration, const FString&, Narration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDMActionsReady, const TArray<FDMAction>&, Actions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDMDiceResolved, const FAbilityCheckResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDMProcessingStateChanged, bool, bIsProcessing);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInventoryChanged, const FString&, ItemName);

UCLASS()
class GREYMAWCHRONICLES_API UDMBrainSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "DM")
    void ProcessPlayerInput(const FString& PlayerInput);

    UFUNCTION(BlueprintCallable, Category = "DM")
    void SetRuleContext(UGCCharacterSheet* InPlayerSheet);

    UFUNCTION(BlueprintCallable, Category = "DM")
    void ResolveParsedResponse(const FDMResponse& Parsed);

    UFUNCTION(BlueprintPure, Category = "DM")
    bool IsProcessing() const { return bIsProcessing; }

    /** Sprint J: Read-only access for save/load system. */
    UFUNCTION(BlueprintPure, Category = "DM")
    UGCCharacterSheet* GetPlayerSheet() const { return PlayerSheet; }

    /** Sprint J: Read-only access for save/load system. */
    UFUNCTION(BlueprintPure, Category = "DM")
    UDMConversationHistory* GetConversationHistory() const { return ConversationHistory; }

    UPROPERTY(BlueprintAssignable, Category = "DM")
    FOnDMNarration OnDMNarration;

    UPROPERTY(BlueprintAssignable, Category = "DM")
    FOnDMActionsReady OnDMActionsReady;

    UPROPERTY(BlueprintAssignable, Category = "DM")
    FOnDMDiceResolved OnDMDiceResolved;

    UPROPERTY(BlueprintAssignable, Category = "DM")
    FOnDMProcessingStateChanged OnDMProcessingStateChanged;

    /** Sprint J: Fired when an item is added to the player's inventory. */
    UPROPERTY(BlueprintAssignable, Category = "DM")
    FOnInventoryChanged OnInventoryChanged;

private:
    void OnOllamaCompletion(bool bSuccess, const FString& ResponseText, float LatencySeconds, FString OriginalPlayerInput);
    void ApplyWorldChanges(const TArray<FDMWorldChange>& WorldChanges);
    EGCSkill ResolveCheckSkill(const FString& CheckType) const;
    bool TryHandleScriptedTavernPrompt(const FString& PlayerInput);
    bool HandleIntentScripted(const FDMIntentResult& Intent, const FString& RawInput);
    void SetProcessingState(bool bNewState);

    UPROPERTY()
    TObjectPtr<UDMPromptBuilder> PromptBuilder;

    UPROPERTY()
    TObjectPtr<UDMIntentClassifier> IntentClassifier;

    UPROPERTY()
    TObjectPtr<UDMNarrationPool> NarrationPool;

    UPROPERTY()
    TObjectPtr<UDMResponseParser> ResponseParser;

    UPROPERTY()
    TObjectPtr<UDMConversationHistory> ConversationHistory;

    UPROPERTY()
    TObjectPtr<UOllamaSubsystem> OllamaSubsystem;

    UPROPERTY()
    TObjectPtr<UDiceRoller> DiceRoller;

    UPROPERTY()
    TObjectPtr<UAbilityCheckResolver> AbilityCheckResolver;

    UPROPERTY()
    TObjectPtr<UCombatResolver> CombatResolver;

    UPROPERTY()
    TObjectPtr<USpellSystem> SpellSystem;

    UPROPERTY()
    TObjectPtr<UGCCharacterSheet> PlayerSheet;

    /** Sprint I: Session-scoped world state for NPC dispositions */
    UPROPERTY()
    TObjectPtr<UDMWorldStateSubsystem> WorldState;

    UPROPERTY(EditAnywhere, Category = "DM|Demo")
    bool bUseTavernScriptedBootstrap = true;

    UPROPERTY()
    bool bIsProcessing = false;

    /** Sprint I: Safety timeout for processing state */
    FTimerHandle ProcessingTimeoutHandle;

    /** Sprint I: Debounce timer to prevent double-submit */
    FTimerHandle DebounceHandle;
};
