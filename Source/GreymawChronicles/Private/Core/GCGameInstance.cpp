#include "Core/GCGameInstance.h"

#include "ActionSystem/ActionDirectorSubsystem.h"
#include "Core/GreymawSaveGame.h"
#include "DungeonMaster/DMBrainSubsystem.h"
#include "DungeonMaster/DMConversationHistory.h"
#include "DungeonMaster/DMWorldStateSubsystem.h"
#include "Gameplay/GCCharacterSheet.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogGCGameInstance, Log, All);

void UGCGameInstance::Init()
{
    Super::Init();
    UE_LOG(LogGCGameInstance, Log, TEXT("GCGameInstance initialized"));

    UDMBrainSubsystem* DMBrain = GetSubsystem<UDMBrainSubsystem>();
    ActionDirector = GetSubsystem<UActionDirectorSubsystem>();

    if (DMBrain)
    {
        UE_LOG(LogGCGameInstance, Log, TEXT("DMBrainSubsystem available"));
        DMBrain->OnDMActionsReady.AddDynamic(this, &UGCGameInstance::HandleDMActionsReady);
    }

    // Sprint J: Auto-save after each action sequence completion
    if (ActionDirector)
    {
        ActionDirector->OnActionSequenceComplete.AddDynamic(this, &UGCGameInstance::HandleActionSequenceComplete);
    }

    // Sprint J: Restore previous session if a save exists
    if (HasSaveGame())
    {
        LoadGame();
        UE_LOG(LogGCGameInstance, Log, TEXT("Loaded saved game state."));
    }
}

UDMBrainSubsystem* UGCGameInstance::GetDMBrainSubsystem() const
{
    return GetSubsystem<UDMBrainSubsystem>();
}

void UGCGameInstance::HandleDMActionsReady(const TArray<FDMAction>& Actions)
{
    if (!ActionDirector)
    {
        ActionDirector = GetSubsystem<UActionDirectorSubsystem>();
    }

    if (ActionDirector)
    {
        ActionDirector->ExecuteDMActions(Actions);
    }
}

void UGCGameInstance::HandleActionSequenceComplete()
{
    SaveGame();
}

bool UGCGameInstance::SaveGame()
{
    UGreymawSaveGame* SaveData = Cast<UGreymawSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UGreymawSaveGame::StaticClass()));
    if (!SaveData)
    {
        return false;
    }

    // 1. Serialize CharacterSheet + ConversationHistory via DMBrain accessors
    UDMBrainSubsystem* DMBrain = GetSubsystem<UDMBrainSubsystem>();
    if (DMBrain)
    {
        if (UGCCharacterSheet* Sheet = DMBrain->GetPlayerSheet())
        {
            SaveData->CharacterSheetJSON = Sheet->ToCompactJSON();
        }

        if (UDMConversationHistory* History = DMBrain->GetConversationHistory())
        {
            SaveData->ConversationHistoryJSON = History->SerializeToJSON();
        }
    }

    // 2. Serialize WorldState
    UDMWorldStateSubsystem* WorldState = GetSubsystem<UDMWorldStateSubsystem>();
    if (WorldState)
    {
        SaveData->WorldStateJSON = WorldState->ToJSON();
    }

    // 3. Timestamp
    SaveData->SaveTimestamp = FDateTime::UtcNow().ToIso8601();

    const bool bSaved = UGameplayStatics::SaveGameToSlot(
        SaveData, UGreymawSaveGame::SaveSlotName, UGreymawSaveGame::UserIndex);

    if (bSaved)
    {
        UE_LOG(LogGCGameInstance, Log, TEXT("Game saved to slot '%s' at %s"),
            *UGreymawSaveGame::SaveSlotName, *SaveData->SaveTimestamp);
    }
    else
    {
        UE_LOG(LogGCGameInstance, Warning, TEXT("Failed to save game."));
    }

    return bSaved;
}

bool UGCGameInstance::LoadGame()
{
    if (!HasSaveGame())
    {
        return false;
    }

    UGreymawSaveGame* SaveData = Cast<UGreymawSaveGame>(
        UGameplayStatics::LoadGameFromSlot(
            UGreymawSaveGame::SaveSlotName, UGreymawSaveGame::UserIndex));
    if (!SaveData)
    {
        return false;
    }

    // 1. Restore CharacterSheet + ConversationHistory
    UDMBrainSubsystem* DMBrain = GetSubsystem<UDMBrainSubsystem>();
    if (DMBrain)
    {
        if (UGCCharacterSheet* Sheet = DMBrain->GetPlayerSheet())
        {
            FString Error;
            Sheet->FromJSON(SaveData->CharacterSheetJSON, Error);
            if (!Error.IsEmpty())
            {
                UE_LOG(LogGCGameInstance, Warning, TEXT("CharacterSheet load error: %s"), *Error);
            }
        }

        if (UDMConversationHistory* History = DMBrain->GetConversationHistory())
        {
            History->DeserializeFromJSON(SaveData->ConversationHistoryJSON);
        }
    }

    // 2. Restore WorldState
    UDMWorldStateSubsystem* WorldState = GetSubsystem<UDMWorldStateSubsystem>();
    if (WorldState)
    {
        WorldState->FromJSON(SaveData->WorldStateJSON);
    }

    UE_LOG(LogGCGameInstance, Log, TEXT("Game loaded from slot '%s' (saved at %s)"),
        *UGreymawSaveGame::SaveSlotName, *SaveData->SaveTimestamp);

    return true;
}

bool UGCGameInstance::HasSaveGame() const
{
    return UGameplayStatics::DoesSaveGameExist(
        UGreymawSaveGame::SaveSlotName, UGreymawSaveGame::UserIndex);
}
