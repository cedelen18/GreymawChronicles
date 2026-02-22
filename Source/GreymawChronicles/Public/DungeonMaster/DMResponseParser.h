#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DungeonMaster/DMTypes.h"
#include "DMResponseParser.generated.h"

UCLASS(BlueprintType)
class GREYMAWCHRONICLES_API UDMResponseParser : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "DM")
    FDMResponse ParseResponse(const FString& RawText) const;

private:
    FString RepairJson(const FString& InJson) const;
    bool IsAllowedAnimationAction(const FString& ActionName) const;
};
