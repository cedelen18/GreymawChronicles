#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GCGameInstance.generated.h"

class UDMBrainSubsystem;

UCLASS()
class GREYMAWCHRONICLES_API UGCGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;

    UFUNCTION(BlueprintPure, Category = "Greymaw|Core")
    UDMBrainSubsystem* GetDMBrainSubsystem() const;
};
