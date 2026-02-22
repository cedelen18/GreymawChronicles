#include "Core/GCGameInstance.h"

#include "ActionSystem/ActionDirectorSubsystem.h"
#include "DungeonMaster/DMBrainSubsystem.h"

void UGCGameInstance::Init()
{
    Super::Init();
    UE_LOG(LogTemp, Log, TEXT("GCGameInstance initialized"));

    UDMBrainSubsystem* DMBrain = GetSubsystem<UDMBrainSubsystem>();
    ActionDirector = GetSubsystem<UActionDirectorSubsystem>();

    if (DMBrain)
    {
        UE_LOG(LogTemp, Log, TEXT("DMBrainSubsystem available"));
        DMBrain->OnDMActionsReady.AddDynamic(this, &UGCGameInstance::HandleDMActionsReady);
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
