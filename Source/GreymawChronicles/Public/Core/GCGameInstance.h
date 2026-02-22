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

private:
    UFUNCTION()
    void HandleDMActionsReady(const TArray<FDMAction>& Actions);

    UPROPERTY()
    TObjectPtr<UActionDirectorSubsystem> ActionDirector;
};
