#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DungeonMaster/DMTypes.h"
#include "GCGameInstance.generated.h"

class UDMBrainSubsystem;
class UActionDirectorSubsystem;
struct FDMAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveLoadFeedback, const FString&, Message);

UCLASS()
class GREYMAWCHRONICLES_API UGCGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;

    UFUNCTION(BlueprintPure, Category = "Greymaw|Core")
    UDMBrainSubsystem* GetDMBrainSubsystem() const;

    /** Sprint J: Save all game state to slot. */
    UFUNCTION(BlueprintCallable, Category = "Greymaw|Save")
    bool SaveGame();

    /** Sprint J: Load game state from slot. */
    UFUNCTION(BlueprintCallable, Category = "Greymaw|Save")
    bool LoadGame();

    /** Sprint J: Check if a save game exists. */
    UFUNCTION(BlueprintPure, Category = "Greymaw|Save")
    bool HasSaveGame() const;

    /** Sprint L: Save to a manual slot (0-2). */
    UFUNCTION(BlueprintCallable, Category = "Greymaw|Save")
    bool SaveToSlot(int32 SlotIndex);

    /** Sprint L: Load from a manual slot (0-2). */
    UFUNCTION(BlueprintCallable, Category = "Greymaw|Save")
    bool LoadFromSlot(int32 SlotIndex);

    /** Sprint L: Check if a manual slot has a save. */
    UFUNCTION(BlueprintPure, Category = "Greymaw|Save")
    bool HasSaveInSlot(int32 SlotIndex) const;

    /** Sprint K: Feedback delegate for save/load events. */
    UPROPERTY(BlueprintAssignable, Category = "Greymaw|Save")
    FOnSaveLoadFeedback OnSaveLoadFeedback;

private:
    UFUNCTION()
    void HandleDMActionsReady(const TArray<FDMAction>& Actions);

    /** Sprint J: Auto-save after each completed action sequence. */
    UFUNCTION()
    void HandleActionSequenceComplete();

    UPROPERTY()
    TObjectPtr<UActionDirectorSubsystem> ActionDirector;

    /** Sprint K: Console commands for manual save/load. */
    TUniquePtr<FAutoConsoleCommand> ManualSaveCmd;
    TUniquePtr<FAutoConsoleCommand> ManualLoadCmd;
};
