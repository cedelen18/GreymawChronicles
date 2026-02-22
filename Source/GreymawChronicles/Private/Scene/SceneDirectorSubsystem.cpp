#include "Scene/SceneDirectorSubsystem.h"

bool UGCSceneDirectorSubsystem::RequestLoadSceneAsset(const UGCSceneTemplateAsset* SceneAsset)
{
    if (!SceneAsset)
    {
        return false;
    }

    return RequestLoadSceneTemplate(SceneAsset->Template);
}

bool UGCSceneDirectorSubsystem::RequestLoadSceneTemplate(const FGCSceneTemplate& SceneTemplate)
{
    if (SceneTemplate.SceneId.IsNone())
    {
        return false;
    }

    ActiveSceneId = SceneTemplate.SceneId;
    OnSceneLoadRequested.Broadcast(SceneTemplate);
    return true;
}

void UGCSceneDirectorSubsystem::UnloadActiveScene()
{
    if (ActiveSceneId.IsNone())
    {
        return;
    }

    const FName PreviousSceneId = ActiveSceneId;
    ActiveSceneId = NAME_None;
    OnSceneUnloaded.Broadcast(PreviousSceneId);
}
