#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DMPromptBuilder.generated.h"

USTRUCT(BlueprintType)
struct FDMPromptContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SceneContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CharacterSheets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AdventureState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ConversationHistory;
};

UCLASS(BlueprintType)
class GREYMAWCHRONICLES_API UDMPromptBuilder : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "DM")
    FString BuildDMPrompt(const FString& PlayerInput, const FDMPromptContext& Context) const;

    UFUNCTION(BlueprintCallable, Category = "DM")
    FString BuildCompanionCombatPrompt(const FString& CombatStateJson) const;

private:
    FString BuildStaticSections() const;
};
