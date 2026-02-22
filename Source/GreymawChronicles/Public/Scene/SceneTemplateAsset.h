#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/World.h"
#include "Scene/SceneTransitionTypes.h"
#include "SceneTemplateAsset.generated.h"

USTRUCT(BlueprintType)
struct GREYMAWCHRONICLES_API FGCSceneTemplate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scene")
    FName SceneId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scene")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scene")
    TSoftObjectPtr<UWorld> World;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scene")
    FGCSceneTransition Transition;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scene")
    bool bPreloadAssets = false;
};

UCLASS(BlueprintType)
class GREYMAWCHRONICLES_API UGCSceneTemplateAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scene")
    FGCSceneTemplate Template;
};
