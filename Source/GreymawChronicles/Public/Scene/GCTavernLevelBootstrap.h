#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GCTavernLevelBootstrap.generated.h"

/**
 * Sprint G: Placed in L_Persistent to spawn floor, lighting,
 * PlayerStart marker, and a "bar" target anchor at runtime.
 * Provides a minimal playable environment without binary assets.
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

	UPROPERTY(EditAnywhere, Category = "Tavern|Floor")
	FVector FloorScale = FVector(20.0f, 20.0f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Tavern|Bar")
	FVector BarLocation = FVector(350.0f, 0.0f, 50.0f);

	UPROPERTY(EditAnywhere, Category = "Tavern|PlayerStart")
	FVector PlayerStartLocation = FVector(0.0f, 0.0f, 100.0f);
};
