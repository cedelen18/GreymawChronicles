#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GCQuestSubsystem.generated.h"

/**
 * Sprint L: Quest status lifecycle.
 */
UENUM(BlueprintType)
enum class EGCQuestStatus : uint8
{
    NotStarted  UMETA(DisplayName = "Not Started"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

/**
 * Sprint L: A single quest entry.
 */
USTRUCT(BlueprintType)
struct FGCQuestEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString QuestId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CurrentObjective;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EGCQuestStatus Status = EGCQuestStatus::NotStarted;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestUpdated, const FString&, QuestId, EGCQuestStatus, NewStatus);

/**
 * Sprint L: Data-driven quest tracking subsystem.
 * Manages quest lifecycle (start, update objective, complete, fail).
 * Supports JSON serialization for save/load integration.
 */
UCLASS()
class GREYMAWCHRONICLES_API UGCQuestSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    /** Start a new quest. No-op if quest already exists. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void StartQuest(const FString& QuestId, const FString& Title, const FString& Description);

    /** Update the current objective text for an active quest. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateObjective(const FString& QuestId, const FString& ObjectiveText);

    /** Mark a quest as completed. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteQuest(const FString& QuestId);

    /** Mark a quest as failed. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(const FString& QuestId);

    /** Get the status of a quest. Returns NotStarted if quest doesn't exist. */
    UFUNCTION(BlueprintPure, Category = "Quest")
    EGCQuestStatus GetQuestStatus(const FString& QuestId) const;

    /** Get all active quests. */
    UFUNCTION(BlueprintPure, Category = "Quest")
    TArray<FGCQuestEntry> GetActiveQuests() const;

    /** Get all quests regardless of status. */
    UFUNCTION(BlueprintPure, Category = "Quest")
    TArray<FGCQuestEntry> GetAllQuests() const;

    /** Get a specific quest entry. Returns nullptr-like default if not found. */
    const FGCQuestEntry* FindQuest(const FString& QuestId) const;

    /** Serialize all quests to JSON for save system. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    FString ToJSON() const;

    /** Restore quests from JSON. Clears existing quests first. */
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FromJSON(const FString& Json);

    /** Fired whenever a quest status changes. */
    UPROPERTY(BlueprintAssignable, Category = "Quest")
    FOnQuestUpdated OnQuestUpdated;

private:
    TMap<FString, FGCQuestEntry> QuestLog;
};
