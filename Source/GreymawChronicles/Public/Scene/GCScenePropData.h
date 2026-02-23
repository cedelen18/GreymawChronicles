#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GCScenePropData.generated.h"

/**
 * Sprint L: Data table row for scene props.
 * Structural data layer so tavern asset import is plug-and-play.
 * GCTavernLevelBootstrap can later read this table instead of
 * hardcoded cube spawns.
 */
USTRUCT(BlueprintType)
struct FGCScenePropRow : public FTableRowBase
{
    GENERATED_BODY()

    /** Unique identifier (e.g. "tavern_bar_counter"). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    FString PropId;

    /** Display name shown in inspections (e.g. "Bar Counter"). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    FString DisplayName;

    /** Mesh asset path. Empty until meshes are imported. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    FString MeshAssetPath;

    /** Default world location for this prop. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    FVector DefaultLocation = FVector::ZeroVector;

    /** Default world rotation for this prop. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    FRotator DefaultRotation = FRotator::ZeroRotator;

    /** Whether this prop can be interacted with (inspect, use). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
    bool bInteractable = false;
};
