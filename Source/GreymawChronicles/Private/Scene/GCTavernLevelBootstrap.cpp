#include "Scene/GCTavernLevelBootstrap.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"

DEFINE_LOG_CATEGORY_STATIC(LogTavernBootstrap, Log, All);

AGCTavernLevelBootstrap::AGCTavernLevelBootstrap()
{
	PrimaryActorTick.bCanEverTick = false;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void AGCTavernLevelBootstrap::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTavernBootstrap, Log, TEXT("Tavern bootstrap starting — spawning floor, lights, bar marker, player start."));

	SpawnFloor();
	SpawnLighting();
	SpawnBarMarker();
	SpawnPlayerStart();
	SpawnNPCs();

	UE_LOG(LogTavernBootstrap, Log, TEXT("Tavern bootstrap complete."));
}

void AGCTavernLevelBootstrap::SpawnFloor()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Spawn a flat cube as the floor
	FActorSpawnParameters Params;
	Params.Name = TEXT("TavernFloor");
	AActor* Floor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, Params);
	if (!Floor)
	{
		UE_LOG(LogTavernBootstrap, Warning, TEXT("Failed to spawn floor actor."));
		return;
	}

	USceneComponent* FloorRoot = NewObject<USceneComponent>(Floor, TEXT("FloorRoot"));
	Floor->SetRootComponent(FloorRoot);
	FloorRoot->RegisterComponent();

	UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(Floor, TEXT("FloorMesh"));
	MeshComp->SetupAttachment(FloorRoot);

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh)
	{
		MeshComp->SetStaticMesh(CubeMesh);
		MeshComp->SetWorldScale3D(FloorScale);
		MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -50.0f));
	}
	else
	{
		UE_LOG(LogTavernBootstrap, Warning, TEXT("Could not load /Engine/BasicShapes/Cube — floor will be invisible."));
	}

	MeshComp->RegisterComponent();
}

void AGCTavernLevelBootstrap::SpawnLighting()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Directional light for basic illumination
	FActorSpawnParameters DirParams;
	DirParams.Name = TEXT("TavernDirectionalLight");
	World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(),
		FVector(0.0f, 0.0f, 800.0f),
		FRotator(-45.0f, -30.0f, 0.0f),
		DirParams);

	// Sky light for ambient fill
	FActorSpawnParameters SkyParams;
	SkyParams.Name = TEXT("TavernSkyLight");
	World->SpawnActor<ASkyLight>(ASkyLight::StaticClass(),
		FVector(0.0f, 0.0f, 600.0f),
		FRotator::ZeroRotator,
		SkyParams);
}

void AGCTavernLevelBootstrap::SpawnBarMarker()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Spawn a visible cube as the bar target anchor (tagged "bar")
	FActorSpawnParameters Params;
	Params.Name = TEXT("BarMarker");
	AActor* Marker = World->SpawnActor<AActor>(AActor::StaticClass(), BarLocation, FRotator::ZeroRotator, Params);
	if (!Marker)
	{
		return;
	}

	USceneComponent* MarkerRoot = NewObject<USceneComponent>(Marker, TEXT("BarRoot"));
	Marker->SetRootComponent(MarkerRoot);
	MarkerRoot->RegisterComponent();

	UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(Marker, TEXT("BarMesh"));
	MeshComp->SetupAttachment(MarkerRoot);

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh)
	{
		MeshComp->SetStaticMesh(CubeMesh);
		MeshComp->SetWorldScale3D(FVector(3.0f, 1.0f, 1.5f));
	}

	MeshComp->RegisterComponent();

	// Tag so ActionDirector can find it as a "bar" target
	Marker->Tags.Add(TEXT("bar"));
}

void AGCTavernLevelBootstrap::SpawnPlayerStart()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Only spawn if no PlayerStart already exists
	if (UGameplayStatics::GetActorOfClass(World, APlayerStart::StaticClass()))
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.Name = TEXT("TavernPlayerStart");
	World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), PlayerStartLocation, FRotator::ZeroRotator, Params);
}

void AGCTavernLevelBootstrap::SpawnNPCs()
{
	// Marta: barkeep, near bar, warm red tint
	SpawnNPCActor(TEXT("marta"), MartaLocation,
		FRotator(0.0f, 180.0f, 0.0f),
		FLinearColor(0.8f, 0.2f, 0.1f));

	// Durgan: old storyteller at a table, blue-grey tint
	SpawnNPCActor(TEXT("durgan"), DurganLocation,
		FRotator(0.0f, 45.0f, 0.0f),
		FLinearColor(0.3f, 0.4f, 0.7f));

	// Kael: mercenary in the corner, dark green tint
	SpawnNPCActor(TEXT("kael"), KaelLocation,
		FRotator(0.0f, -90.0f, 0.0f),
		FLinearColor(0.15f, 0.5f, 0.2f));
}

AActor* AGCTavernLevelBootstrap::SpawnNPCActor(
	const FString& NPCName,
	const FVector& Location,
	const FRotator& Rotation,
	const FLinearColor& TintColor)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.Name = FName(*FString::Printf(TEXT("NPC_%s"), *NPCName));

	AActor* NPC = World->SpawnActor<AActor>(
		AActor::StaticClass(), Location, Rotation, Params);
	if (!NPC)
	{
		return nullptr;
	}

	// Root component
	USceneComponent* Root = NewObject<USceneComponent>(NPC, TEXT("NPCRoot"));
	NPC->SetRootComponent(Root);
	Root->RegisterComponent();

	// Skeletal mesh — UE5 mannequin as placeholder
	USkeletalMeshComponent* MeshComp =
		NewObject<USkeletalMeshComponent>(NPC, TEXT("NPCMesh"));
	MeshComp->SetupAttachment(Root);

	// Try project-local path first, then engine path
	USkeletalMesh* Mannequin = LoadObject<USkeletalMesh>(nullptr,
		TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny"));
	if (!Mannequin)
	{
		Mannequin = LoadObject<USkeletalMesh>(nullptr,
			TEXT("/Engine/Characters/Mannequins/Meshes/SKM_Manny.SKM_Manny"));
	}

	if (Mannequin)
	{
		MeshComp->SetSkeletalMesh(Mannequin);

		// Dynamic material instance for color tint (best-effort)
		if (MeshComp->GetNumMaterials() > 0)
		{
			UMaterialInstanceDynamic* DynMat =
				MeshComp->CreateDynamicMaterialInstance(0);
			if (DynMat)
			{
				DynMat->SetVectorParameterValue(TEXT("BodyColor"), TintColor);
				DynMat->SetVectorParameterValue(TEXT("Base Color"), TintColor);
				DynMat->SetVectorParameterValue(TEXT("Tint"), TintColor);
			}
		}
	}
	else
	{
		UE_LOG(LogTavernBootstrap, Warning,
			TEXT("Mannequin mesh not found for NPC '%s'. "
				"NPC will exist as invisible tagged actor."), *NPCName);
	}

	MeshComp->RegisterComponent();

	// Tag for ResolveActor lookup
	NPC->Tags.Add(FName(*NPCName));

	UE_LOG(LogTavernBootstrap, Log,
		TEXT("Spawned NPC '%s' at %s"), *NPCName, *Location.ToString());

	return NPC;
}
