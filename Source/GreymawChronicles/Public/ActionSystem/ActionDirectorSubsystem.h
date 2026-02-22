#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DungeonMaster/DMTypes.h"
#include "ActionDirectorSubsystem.generated.h"

class UCinematicCameraSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActionSequenceComplete);

UCLASS()
class GREYMAWCHRONICLES_API UActionDirectorSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Greymaw|Action")
    void ExecuteDMActions(const TArray<FDMAction>& InActions);

    UPROPERTY(BlueprintAssignable, Category = "Greymaw|Action")
    FOnActionSequenceComplete OnActionSequenceComplete;

private:
    void BeginNextAction();
    void CompleteCurrentAction();
    void TickMoveStep();

    AActor* ResolveActor(const FString& ActorId) const;
    bool TryPlayAnimation(AActor* Actor, const FString& AnimationRef) const;
    void BeginMoveAction(AActor* Actor, const FDMAction& Action);

    UPROPERTY()
    TObjectPtr<UCinematicCameraSubsystem> CameraSubsystem;

    UPROPERTY()
    TArray<FDMAction> PendingActions;

    int32 ActionIndex = INDEX_NONE;

    UPROPERTY()
    TObjectPtr<AActor> MovingActor;

    FVector MoveStart = FVector::ZeroVector;
    FVector MoveEnd = FVector::ZeroVector;
    float MoveDuration = 0.0f;
    double MoveStartTime = 0.0;

    FTimerHandle DelayHandle;
    FTimerHandle MoveTickHandle;
};