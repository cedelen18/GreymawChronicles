#include "Core/GCGameInstance.h"

#include "DungeonMaster/DMBrainSubsystem.h"

void UGCGameInstance::Init()
{
    Super::Init();
    UE_LOG(LogTemp, Log, TEXT("GCGameInstance initialized"));

    if (GetSubsystem<UDMBrainSubsystem>())
    {
        UE_LOG(LogTemp, Log, TEXT("DMBrainSubsystem available"));
    }
}

UDMBrainSubsystem* UGCGameInstance::GetDMBrainSubsystem() const
{
    return GetSubsystem<UDMBrainSubsystem>();
}
