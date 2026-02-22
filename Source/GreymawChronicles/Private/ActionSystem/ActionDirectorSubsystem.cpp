#include "ActionSystem/ActionDirectorSubsystem.h"

#include "ActionSystem/CinematicCameraSubsystem.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogActionDirector, Log, All);

void UActionDirectorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UActionDirectorSubsystem::ExecuteDMActions(const TArray<FDMAction>& InActions)
{
    PendingActions = InActions;
    ActionIndex = INDEX_NONE;

    if (PendingActions.IsEmpty())
    {
        OnActionSequenceComplete.Broadcast();
        return;
    }

    BeginNextAction();
}

void UActionDirectorSubsystem::BeginNextAction()
{
    ++ActionIndex;
    if (!PendingActions.IsValidIndex(ActionIndex))
    {
        OnActionSequenceComplete.Broadcast();
        return;
    }

    const FDMAction& Action = PendingActions[ActionIndex];
    AActor* Actor = ResolveActor(Action.Actor);

    if (UWorld* World = GetWorld())
    {
        CameraSubsystem = World->GetSubsystem<UCinematicCameraSubsystem>();
        if (CameraSubsystem && Actor)
        {
            const FString Lower = Action.Action.ToLower();
            const EGreymawCameraMode CameraMode = Lower.Contains(TEXT("combat")) ? EGreymawCameraMode::Combat
                : (Lower.Contains(TEXT("close")) || Lower.Contains(TEXT("talk"))) ? EGreymawCameraMode::Close
                : (Lower.Contains(TEXT("move")) || Lower.Contains(TEXT("walk"))) ? EGreymawCameraMode::Medium
                : EGreymawCameraMode::Establishing;
            CameraSubsystem->SwitchCameraMode(CameraMode, Actor, 0.6f);
        }
    }

    if (Actor && Action.bHasMoveTarget)
    {
        BeginMoveAction(Actor, Action);
        return;
    }

    if (Actor && !Action.Animation.IsEmpty())
    {
        TryPlayAnimation(Actor, Action.Animation);
    }

    const float Delay = FMath::Max(0.0f, Action.DelaySeconds);
    if (Delay > 0.0f && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(DelayHandle, this, &UActionDirectorSubsystem::CompleteCurrentAction, Delay, false);
    }
    else
    {
        CompleteCurrentAction();
    }
}

void UActionDirectorSubsystem::BeginMoveAction(AActor* Actor, const FDMAction& Action)
{
    if (!GetWorld() || !Actor)
    {
        UE_LOG(LogActionDirector, Warning, TEXT("BeginMoveAction skipped: %s."), !Actor ? TEXT("null actor") : TEXT("no world"));
        CompleteCurrentAction();
        return;
    }

    MovingActor = Actor;
    MoveStart = Actor->GetActorLocation();
    MoveEnd = Action.MoveTarget;

    const float Distance = FVector::Distance(MoveStart, MoveEnd);
    MoveDuration = FMath::Max(0.1f, Distance / FMath::Max(1.0f, Action.MoveSpeedUnitsPerSecond));
    MoveStartTime = GetWorld()->GetTimeSeconds();

    GetWorld()->GetTimerManager().SetTimer(MoveTickHandle, this, &UActionDirectorSubsystem::TickMoveStep, 0.02f, true);

    // Safety timeout: force-complete if move takes longer than 10 seconds (stuck prevention)
    static constexpr float MaxMoveDuration = 10.0f;
    if (MoveDuration > MaxMoveDuration)
    {
        UE_LOG(LogActionDirector, Warning, TEXT("Move duration %.1fs exceeds safety limit; clamping to %.0fs."), MoveDuration, MaxMoveDuration);
        MoveDuration = MaxMoveDuration;
    }
}

void UActionDirectorSubsystem::TickMoveStep()
{
    UWorld* World = GetWorld();
    if (!World || !MovingActor)
    {
        CompleteCurrentAction();
        return;
    }

    const float Elapsed = static_cast<float>(World->GetTimeSeconds() - MoveStartTime);
    const float Alpha = FMath::Clamp(Elapsed / MoveDuration, 0.0f, 1.0f);
    const FVector NewLocation = FMath::Lerp(MoveStart, MoveEnd, Alpha);
    MovingActor->SetActorLocation(NewLocation);

    if (Alpha >= 1.0f)
    {
        World->GetTimerManager().ClearTimer(MoveTickHandle);
        CompleteCurrentAction();
    }
}

void UActionDirectorSubsystem::CompleteCurrentAction()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(DelayHandle);
        World->GetTimerManager().ClearTimer(MoveTickHandle);
    }

    MovingActor = nullptr;
    BeginNextAction();
}

AActor* UActionDirectorSubsystem::ResolveActor(const FString& ActorId) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    if (ActorId.IsEmpty() || ActorId.Equals(TEXT("player"), ESearchCase::IgnoreCase))
    {
        return UGameplayStatics::GetPlayerPawn(World, 0);
    }

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Candidate = *It;
        if (!Candidate)
        {
            continue;
        }

        if (Candidate->ActorHasTag(FName(*ActorId)) || Candidate->GetName().Equals(ActorId, ESearchCase::IgnoreCase))
        {
            return Candidate;
        }
    }

    UE_LOG(LogActionDirector, Warning, TEXT("Could not resolve action actor '%s'; falling back to player."), *ActorId);
    return UGameplayStatics::GetPlayerPawn(World, 0);
}

bool UActionDirectorSubsystem::TryPlayAnimation(AActor* Actor, const FString& AnimationRef) const
{
    const ACharacter* Character = Cast<ACharacter>(Actor);
    USkeletalMeshComponent* Mesh = Character ? Character->GetMesh() : Actor->FindComponentByClass<USkeletalMeshComponent>();
    if (!Mesh)
    {
        UE_LOG(LogActionDirector, Warning, TEXT("Action animation '%s' skipped: actor '%s' has no mesh."), *AnimationRef, *GetNameSafe(Actor));
        return false;
    }

    UAnimInstance* AnimInstance = Mesh->GetAnimInstance();
    if (!AnimInstance)
    {
        UE_LOG(LogActionDirector, Warning, TEXT("Action animation '%s' skipped: no anim instance on '%s'."), *AnimationRef, *GetNameSafe(Actor));
        return false;
    }

    UAnimMontage* Montage = LoadObject<UAnimMontage>(nullptr, *AnimationRef);
    if (!Montage)
    {
        UE_LOG(LogActionDirector, Warning, TEXT("Action animation '%s' not found. Continuing without montage."), *AnimationRef);
        return false;
    }

    AnimInstance->Montage_Play(Montage);
    return true;
}