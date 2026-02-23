#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GCAudioSubsystem.generated.h"

class USoundBase;

/**
 * Sprint L: Audio event types.
 */
UENUM(BlueprintType)
enum class EGCAudioEvent : uint8
{
    UI_ButtonClick          UMETA(DisplayName = "UI Button Click"),
    UI_SaveConfirm          UMETA(DisplayName = "UI Save Confirm"),
    UI_QuestUpdate          UMETA(DisplayName = "UI Quest Update"),
    Combat_DiceRoll         UMETA(DisplayName = "Combat Dice Roll"),
    Combat_Hit              UMETA(DisplayName = "Combat Hit"),
    Combat_Miss             UMETA(DisplayName = "Combat Miss"),
    Combat_CriticalHit      UMETA(DisplayName = "Combat Critical Hit"),
    Combat_Victory          UMETA(DisplayName = "Combat Victory"),
    Combat_Defeat           UMETA(DisplayName = "Combat Defeat"),
    Ambience_Tavern         UMETA(DisplayName = "Ambience Tavern"),
    Ambience_Trail          UMETA(DisplayName = "Ambience Trail"),
    Narration_TypewriterTick UMETA(DisplayName = "Narration Typewriter"),
    Narration_NewScene      UMETA(DisplayName = "Narration New Scene"),
    Music_TavernTheme       UMETA(DisplayName = "Music Tavern"),
    Music_CombatTheme       UMETA(DisplayName = "Music Combat"),
    Music_ExplorationTheme  UMETA(DisplayName = "Music Exploration")
};

/**
 * Sprint L: Code-driven audio subsystem.
 * Uses UE5 built-in USoundBase/PlaySound2D.
 * Under NullRHI: PlaySound2D is a no-op, so tests pass headless.
 * Sounds are soft-referenced — null sounds handled gracefully.
 */
UCLASS()
class GREYMAWCHRONICLES_API UGCAudioSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    /** Play a registered audio event. No-op if no sound registered. */
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayEvent(EGCAudioEvent Event);

    /** Stop a playing audio event (if looping). */
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopEvent(EGCAudioEvent Event);

    /** Register a sound asset for an audio event. */
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterSoundForEvent(EGCAudioEvent Event, USoundBase* Sound);

    /** Set master volume multiplier (0.0 - 1.0). */
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);

    /** Get the current master volume. */
    UFUNCTION(BlueprintPure, Category = "Audio")
    float GetMasterVolume() const { return MasterVolume; }

private:
    UPROPERTY()
    TMap<EGCAudioEvent, TObjectPtr<USoundBase>> EventSoundMap;

    float MasterVolume = 1.0f;
};
