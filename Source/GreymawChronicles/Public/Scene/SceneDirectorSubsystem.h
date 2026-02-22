#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Scene/SceneTemplateAsset.h"
#include "SceneDirectorSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGCSceneTemplateLoadRequested, const FGCSceneTemplate&, SceneTemplate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGCSceneIdEvent, FName, SceneId);

UCLASS()
class GREYMAWCHRONICLES_API UGCSceneDirectorSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Scene")
    bool RequestLoadSceneAsset(const UGCSceneTemplateAsset* SceneAsset);

    UFUNCTION(BlueprintCallable, Category = "Scene")
    bool RequestLoadSceneTemplate(const FGCSceneTemplate& SceneTemplate);

    UFUNCTION(BlueprintCallable, Category = "Scene")
    void UnloadActiveScene();

    UFUNCTION(BlueprintPure, Category = "Scene")
    FName GetActiveSceneId() const { return ActiveSceneId; }

    UPROPERTY(BlueprintAssignable, Category = "Scene")
    FGCSceneTemplateLoadRequested OnSceneLoadRequested;

    UPROPERTY(BlueprintAssignable, Category = "Scene")
    FGCSceneIdEvent OnSceneUnloaded;

private:
    UPROPERTY(Transient)
    FName ActiveSceneId = NAME_None;
};
