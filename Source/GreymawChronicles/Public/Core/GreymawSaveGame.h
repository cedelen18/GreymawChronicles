#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GreymawSaveGame.generated.h"

/**
 * Sprint J: Persists CharacterSheet, WorldState, and ConversationHistory
 * across play sessions via UGameplayStatics::SaveGameToSlot/LoadGameFromSlot.
 */
UCLASS()
class GREYMAWCHRONICLES_API UGreymawSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    /** Slot name used for auto-save/load. */
    static const FString SaveSlotName;
    static constexpr int32 UserIndex = 0;

    UPROPERTY()
    FString CharacterSheetJSON;

    UPROPERTY()
    FString WorldStateJSON;

    UPROPERTY()
    FString ConversationHistoryJSON;

    UPROPERTY()
    FString SaveTimestamp;
};
