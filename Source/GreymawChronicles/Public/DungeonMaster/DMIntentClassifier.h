#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DMIntentClassifier.generated.h"

/**
 * Intent buckets for player input classification.
 * Allows fuzzy matching instead of exact keyword matching.
 */
UENUM(BlueprintType)
enum class EDMIntent : uint8
{
    Look        UMETA(DisplayName = "Look / Observe"),
    Move        UMETA(DisplayName = "Move / Go"),
    Talk        UMETA(DisplayName = "Talk / Speak / Ask"),
    Inspect     UMETA(DisplayName = "Inspect / Examine / Read"),
    Challenge   UMETA(DisplayName = "Challenge / Fight / Contest"),
    Help        UMETA(DisplayName = "Help / Assist / Heal"),
    Use         UMETA(DisplayName = "Use / Interact / Open"),
    Order       UMETA(DisplayName = "Order / Buy / Purchase"),
    Steal       UMETA(DisplayName = "Steal / Pickpocket"),
    Listen      UMETA(DisplayName = "Listen / Eavesdrop"),
    Persuade    UMETA(DisplayName = "Persuade / Convince / Charm"),
    Rest        UMETA(DisplayName = "Rest / Relax / Meditate"),
    Gamble      UMETA(DisplayName = "Gamble / Bet / Wager"),
    Accept      UMETA(DisplayName = "Accept / Agree / Volunteer"),
    Report      UMETA(DisplayName = "Report / Inform / Share"),
    Unknown     UMETA(DisplayName = "Unknown / Fallback")
};

USTRUCT(BlueprintType)
struct FDMIntentResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EDMIntent Intent = EDMIntent::Unknown;

    /** Best-guess subject extracted from the input (e.g. "marta", "bar", "quest board"). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString Subject;

    /** Confidence 0.0-1.0.  1.0 = exact keyword match, 0.5+ = fuzzy match. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Confidence = 0.0f;
};

/**
 * Sprint H: Classifies free-form player input into intent buckets with
 * fuzzy keyword matching.  Replaces brittle exact-string checks.
 */
UCLASS(BlueprintType)
class GREYMAWCHRONICLES_API UDMIntentClassifier : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "DM|Intent")
    FDMIntentResult Classify(const FString& PlayerInput) const;

private:
    struct FIntentRule
    {
        EDMIntent Intent;
        TArray<FString> Keywords;
        float BaseConfidence;
    };

    static TArray<FIntentRule> BuildRules();
    FString ExtractSubject(const FString& Input, const TArray<FString>& ConsumedKeywords) const;
};
