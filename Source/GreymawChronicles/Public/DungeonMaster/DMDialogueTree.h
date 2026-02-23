#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DungeonMaster/DMTypes.h"
#include "DMDialogueTree.generated.h"

class UDMWorldStateSubsystem;

/**
 * Sprint L: A single dialogue option within a dialogue node.
 */
USTRUCT(BlueprintType)
struct FGCDialogueOption
{
    GENERATED_BODY()

    /** Display text shown to the player (e.g. "Tell me about the ruins"). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString OptionText;

    /** State requirement: category (empty = no requirement). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RequiredStateCategory;

    /** State requirement: key. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RequiredStateKey;

    /** State requirement: value. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RequiredStateValue;

    /** Narration text when this option is chosen. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ResponseNarration;

    /** World changes applied when this option is chosen. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDMWorldChange> WorldChanges;

    /** Next node to transition to (empty = end conversation). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString NextNodeId;
};

/**
 * Sprint L: A dialogue node — one "screen" of NPC dialogue.
 */
USTRUCT(BlueprintType)
struct FGCDialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString NodeId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString NPCGreeting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGCDialogueOption> Options;
};

/**
 * Sprint L: State-aware branching dialogue system.
 * Options filter based on WorldState conditions.
 */
UCLASS(BlueprintType)
class GREYMAWCHRONICLES_API UDMDialogueTree : public UObject
{
    GENERATED_BODY()

public:
    /** Register a dialogue node. Overwrites if NodeId already exists. */
    void RegisterNode(const FGCDialogueNode& Node);

    /** Get a node by ID. Returns empty node if not found. */
    FGCDialogueNode GetNode(const FString& NodeId) const;

    /** Get available options for a node, filtering by WorldState requirements. */
    TArray<FGCDialogueOption> GetAvailableOptions(const FString& NodeId, const UDMWorldStateSubsystem* WorldState) const;

    /** Check if a node exists. */
    bool HasNode(const FString& NodeId) const;

    /** Populate Marta's dialogue content. */
    void PopulateMartaDialogue();

    /** Populate Durgan's dialogue content. */
    void PopulateDurganDialogue();

private:
    TMap<FString, FGCDialogueNode> Nodes;
};
