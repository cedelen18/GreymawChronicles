#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DMWorldStateSubsystem.generated.h"

/**
 * Sprint I: Session-scoped key-value state store for NPC dispositions,
 * world flags, and other stateful consequences that persist within a
 * single play session.
 *
 * Storage: Category -> Key -> Value
 * Example: "npc_disposition" -> "marta" -> "friendly"
 */
UCLASS()
class GREYMAWCHRONICLES_API UDMWorldStateSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    /** Set an arbitrary state value. Overwrites if already present. */
    UFUNCTION(BlueprintCallable, Category = "DM|WorldState")
    void SetState(const FString& Category, const FString& Key, const FString& Value);

    /** Get a state value. Returns empty string if not found. */
    UFUNCTION(BlueprintPure, Category = "DM|WorldState")
    FString GetState(const FString& Category, const FString& Key) const;

    /** Check whether a state entry exists. */
    UFUNCTION(BlueprintPure, Category = "DM|WorldState")
    bool HasState(const FString& Category, const FString& Key) const;

    /** Remove a single state entry. Returns true if it existed. */
    UFUNCTION(BlueprintCallable, Category = "DM|WorldState")
    bool ClearState(const FString& Category, const FString& Key);

private:
    /** Nested map: Category -> (Key -> Value) */
    TMap<FString, TMap<FString, FString>> StateStore;
};
