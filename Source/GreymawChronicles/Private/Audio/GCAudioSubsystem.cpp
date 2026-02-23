#include "Audio/GCAudioSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogGCAudio, Log, All);

void UGCAudioSubsystem::PlayEvent(EGCAudioEvent Event)
{
    TObjectPtr<USoundBase>* FoundSound = EventSoundMap.Find(Event);
    if (!FoundSound || !(*FoundSound))
    {
        UE_LOG(LogGCAudio, Verbose, TEXT("No sound registered for audio event %d. Skipping."), static_cast<int32>(Event));
        return;
    }

    UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
    if (World)
    {
        UGameplayStatics::PlaySound2D(World, *FoundSound, MasterVolume);
    }
}

void UGCAudioSubsystem::StopEvent(EGCAudioEvent Event)
{
    // For non-looping sounds, stopping is a no-op.
    // Looping sounds would require UAudioComponent tracking — future enhancement.
    UE_LOG(LogGCAudio, Verbose, TEXT("StopEvent called for event %d."), static_cast<int32>(Event));
}

void UGCAudioSubsystem::RegisterSoundForEvent(EGCAudioEvent Event, USoundBase* Sound)
{
    if (Sound)
    {
        EventSoundMap.Add(Event, Sound);
        UE_LOG(LogGCAudio, Log, TEXT("Registered sound '%s' for event %d."), *Sound->GetName(), static_cast<int32>(Event));
    }
}

void UGCAudioSubsystem::SetMasterVolume(float Volume)
{
    MasterVolume = FMath::Clamp(Volume, 0.0f, 1.0f);
}
