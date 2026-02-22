#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DungeonMaster/DMTypes.h"
#include "DMBrainSubsystem.generated.h"

class UOllamaSubsystem;
class UDMPromptBuilder;
class UDMResponseParser;
class UDMConversationHistory;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDMNarration, const FString&, Narration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDMActionsReady, const TArray<FDMAction>&, Actions);

UCLASS()
class GREYMAWCHRONICLES_API UDMBrainSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "DM")
    void ProcessPlayerInput(const FString& PlayerInput);

    UPROPERTY(BlueprintAssignable, Category = "DM")
    FOnDMNarration OnDMNarration;

    UPROPERTY(BlueprintAssignable, Category = "DM")
    FOnDMActionsReady OnDMActionsReady;

private:
    void OnOllamaCompletion(bool bSuccess, const FString& ResponseText, float LatencySeconds, FString OriginalPlayerInput);

    UPROPERTY()
    TObjectPtr<UDMPromptBuilder> PromptBuilder;

    UPROPERTY()
    TObjectPtr<UDMResponseParser> ResponseParser;

    UPROPERTY()
    TObjectPtr<UDMConversationHistory> ConversationHistory;

    UPROPERTY()
    TObjectPtr<UOllamaSubsystem> OllamaSubsystem;
};
