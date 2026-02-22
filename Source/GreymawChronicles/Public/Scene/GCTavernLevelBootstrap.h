#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GCTavernLevelBootstrap.generated.h"

/**
 * Sprint G: Placed in L_Persistent to spawn floor, lighting,
 * PlayerStart marker, and a "bar" target anchor at runtime.
 * Sprint J: Also spawns 3 NPC actors (Marta, Durgan, Kael)
 * with UE5 mannequin meshes and tag-based lookup.
 */
UCLASS()
class GREYMAWCHRONICLES_API AGCTavernLevelBootstrap : public AActor
{
	GENERATED_BODY()

public:
	AGCTavernLevelBootstrap();

protected:
	virtual void BeginPlay() override;

private:
	void SpawnFloor();
	void SpawnLighting();
	void SpawnBarMarker();
	void SpawnPlayerStart();

	/** Sprint J: Spawn all tavern NPCs. */
	void SpawnNPCs();

	/** Sprint J: Spawn a single NPC actor with skeletal mesh and tag. */
	AActor* SpawnNPCActor(
		const FString& NPCName,
		const FVector& Location,
		const FRotator& Rotation,
		const FLinearColor& TintColor);

	UPROPERTY(EditAnywhere, Category = "Tavern|Floor")
	FVector FloorScale = FVector(20.0f, 20.0f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Tavern|Bar")
	FVector BarLocation = FVector(350.0f, 0.0f, 50.0f);

	UPROPERTY(EditAnywhere, Category = "Tavern|PlayerStart")
	FVector PlayerStartLocation = FVector(0.0f, 0.0f, 100.0f);

	UPROPERTY(EditAnywhere, Category = "Tavern|NPCs")
	FVector MartaLocation = FVector(320.0f, 30.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "Tavern|NPCs")
	FVector DurganLocation = FVector(0.0f, -200.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "Tavern|NPCs")
	FVector KaelLocation = FVector(-150.0f, 180.0f, 0.0f);
};
