#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DungeonMaster/DMTypes.h"
#include "GCGameInstance.generated.h"

class UDMBrainSubsystem;
class UActionDirectorSubsystem;
struct FDMAction;

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

private:
    UFUNCTION()
    void HandleDMActionsReady(const TArray<FDMAction>& Actions);

    /** Sprint J: Auto-save after each completed action sequence. */
    UFUNCTION()
    void HandleActionSequenceComplete();

    UPROPERTY()
    TObjectPtr<UActionDirectorSubsystem> ActionDirector;
};
