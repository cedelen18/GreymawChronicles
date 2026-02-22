#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DMConversationHistory.generated.h"

USTRUCT(BlueprintType)
struct FDMExchange
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PlayerInput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DMResponseSummary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SceneID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime TimestampUtc;
};

UCLASS(BlueprintType)
class GREYMAWCHRONICLES_API UDMConversationHistory : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "DM")
    void AddExchange(const FDMExchange& Exchange);

    UFUNCTION(BlueprintCallable, Category = "DM")
    TArray<FDMExchange> GetRecentExchanges() const;

    UFUNCTION(BlueprintCallable, Category = "DM")
    FString ToPromptString() const;

    UFUNCTION(BlueprintCallable, Category = "DM")
    void Clear();

    UFUNCTION(BlueprintCallable, Category = "DM")
    FString SerializeToJSON() const;

    UFUNCTION(BlueprintCallable, Category = "DM")
    bool DeserializeFromJSON(const FString& JsonText);

private:
    UPROPERTY(EditAnywhere, Category = "DM")
    int32 Capacity = 15;

    UPROPERTY()
    TArray<FDMExchange> Exchanges;
};
