#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DMNarrationPool.generated.h"

/**
 * Sprint H: Provides randomized narration variants to reduce
 * the scripted feel.  Each "slot" has multiple lines; PickRandom
 * avoids immediate repeats.
 */
UCLASS(BlueprintType)
class GREYMAWCHRONICLES_API UDMNarrationPool : public UObject
{
    GENERATED_BODY()

public:
    /** Register a set of variant lines under a named slot. */
    void RegisterSlot(const FString& SlotKey, const TArray<FString>& Lines);

    /** Pick a random line from the slot, avoiding the last-used line. */
    FString PickRandom(const FString& SlotKey) const;

    /** Populate the default tavern narration variants. */
    void PopulateTavernDefaults();

    /** Sprint L: Populate trail scene narration variants. */
    void PopulateTrailDefaults();

    /** Sprint L: Populate combat narration variants. */
    void PopulateCombatDefaults();

    /** Sprint L: Populate quest narration variants. */
    void PopulateQuestDefaults();

private:
    TMap<FString, TArray<FString>> Pool;
    mutable TMap<FString, int32> LastUsedIndex;
};
