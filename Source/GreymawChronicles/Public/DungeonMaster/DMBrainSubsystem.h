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
class UCombatResolver;
class USpellSystem;
class UDiceRoller;
class UGCCharacterSheet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDMNarration, const FString&, Narration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDMActionsReady, const TArray<FDMAction>&, Actions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDMDiceResolved, const FAbilityCheckResult&, Result);

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

    UPROPERTY(BlueprintAssignable, Category = "DM")
    FOnDMNarration OnDMNarration;

    UPROPERTY(BlueprintAssignable, Category = "DM")
    FOnDMActionsReady OnDMActionsReady;

    UPROPERTY(BlueprintAssignable, Category = "DM")
    FOnDMDiceResolved OnDMDiceResolved;

private:
    void OnOllamaCompletion(bool bSuccess, const FString& ResponseText, float LatencySeconds, FString OriginalPlayerInput);
    void ApplyWorldChanges(const TArray<FDMWorldChange>& WorldChanges);
    EGCSkill ResolveCheckSkill(const FString& CheckType) const;

    UPROPERTY()
    TObjectPtr<UDMPromptBuilder> PromptBuilder;

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
};
