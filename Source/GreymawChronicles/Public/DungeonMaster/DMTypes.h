#pragma once

#include "CoreMinimal.h"
#include "DMTypes.generated.h"

USTRUCT(BlueprintType)
struct FDMAction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Action;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Target;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Animation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasMoveTarget = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector MoveTarget = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MoveSpeedUnitsPerSecond = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DelaySeconds = 0.0f;
};

USTRUCT(BlueprintType)
struct FDMWorldChange
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Key;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Value;
};

USTRUCT(BlueprintType)
struct FDMCheckRequired
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCheckRequired = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CheckType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DC = 0;
};

USTRUCT(BlueprintType)
struct FDMOutcomeBranch
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Narration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDMAction> Actions;
};

USTRUCT(BlueprintType)
struct FDMSceneChange
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSceneChangeRequested = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SceneId;
};

USTRUCT(BlueprintType)
struct FDMResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bValid = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Error;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Narration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDMAction> Actions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDMWorldChange> WorldChanges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDMCheckRequired Check;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDMOutcomeBranch SuccessBranch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDMOutcomeBranch FailureBranch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDMSceneChange SceneChange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CompanionReaction;
};
