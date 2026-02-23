#include "Core/GreymawSaveGame.h"

const FString UGreymawSaveGame::SaveSlotName = TEXT("GreymawAutoSave");

FString UGreymawSaveGame::MakeSlotName(int32 SlotIndex)
{
    return FString::Printf(TEXT("GreymawSlot_%d"), FMath::Clamp(SlotIndex, 0, 2));
}
