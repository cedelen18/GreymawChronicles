#include "ActionSystem/CinematicCameraSubsystem.h"

#include "Camera/CameraActor.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void UCinematicCameraSubsystem::SwitchCameraMode(EGreymawCameraMode NewMode, AActor* FocusActor, float BlendTime)
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    AActor* CameraActor = EnsureCameraActor();
    if (!PC || !CameraActor)
    {
        return;
    }

    FVector NewLocation = FVector::ZeroVector;
    FRotator NewRotation = FRotator::ZeroRotator;
    ComputeTransform(NewMode, FocusActor, NewLocation, NewRotation);

    CameraActor->SetActorLocationAndRotation(NewLocation, NewRotation);
    PC->SetViewTargetWithBlend(CameraActor, FMath::Max(0.05f, BlendTime));
}

AActor* UCinematicCameraSubsystem::EnsureCameraActor()
{
    if (ManagedCameraActor)
    {
        return ManagedCameraActor.Get();
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    ACameraActor* Spawned = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), FVector(0, 0, 220), FRotator(-15, 0, 0));
    ManagedCameraActor = Spawned;
    return Spawned;
}

void UCinematicCameraSubsystem::ComputeTransform(EGreymawCameraMode Mode, AActor* FocusActor, FVector& OutLocation, FRotator& OutRotation) const
{
    const FVector Focus = FocusActor ? FocusActor->GetActorLocation() : FVector::ZeroVector;
    const FVector Forward = FocusActor ? FocusActor->GetActorForwardVector() : FVector::ForwardVector;

    FVector Offset(0, 0, 160);
    switch (Mode)
    {
    case EGreymawCameraMode::Establishing:
        Offset = (-Forward * 700.0f) + FVector(0, 0, 420.0f);
        break;
    case EGreymawCameraMode::Medium:
        Offset = (-Forward * 360.0f) + FVector(0, 0, 220.0f);
        break;
    case EGreymawCameraMode::Close:
        Offset = (-Forward * 175.0f) + FVector(0, 0, 170.0f);
        break;
    case EGreymawCameraMode::Combat:
        Offset = (-Forward * 520.0f) + FVector(0, 220.0f, 280.0f);
        break;
    }

    OutLocation = Focus + Offset;
    OutRotation = (Focus - OutLocation).Rotation();
}