#pragma once

#include "CoreMinimal.h"
#include "SceneTransitionTypes.generated.h"

UENUM(BlueprintType)
enum class EGCSceneTransitionType : uint8
{
    Cut UMETA(DisplayName = "Cut"),
    Fade UMETA(DisplayName = "Fade"),
    Hold UMETA(DisplayName = "Hold")
};

USTRUCT(BlueprintType)
struct GREYMAWCHRONICLES_API FGCSceneTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
    EGCSceneTransitionType Type = EGCSceneTransitionType::Fade;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene", meta = (ClampMin = "0.0"))
    float DurationSeconds = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scene")
    FText LoadingText;
};
