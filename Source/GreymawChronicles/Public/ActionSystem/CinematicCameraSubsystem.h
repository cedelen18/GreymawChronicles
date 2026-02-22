#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CinematicCameraSubsystem.generated.h"

UENUM(BlueprintType)
enum class EGreymawCameraMode : uint8
{
    Establishing,
    Medium,
    Close,
    Combat
};

UCLASS()
class GREYMAWCHRONICLES_API UCinematicCameraSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Greymaw|Camera")
    void SwitchCameraMode(EGreymawCameraMode NewMode, AActor* FocusActor, float BlendTime = 0.6f);

private:
    AActor* EnsureCameraActor();
    void ComputeTransform(EGreymawCameraMode Mode, AActor* FocusActor, FVector& OutLocation, FRotator& OutRotation) const;

    UPROPERTY()
    TObjectPtr<AActor> ManagedCameraActor;
};