#include "Scene/SceneDirectorSubsystem.h"
#include "DungeonMaster/DMWorldStateSubsystem.h"

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

    // Sprint L: Sync WorldState with active scene ID
    if (UGameInstance* GI = GetGameInstance())
    {
        if (UDMWorldStateSubsystem* WorldState = GI->GetSubsystem<UDMWorldStateSubsystem>())
        {
            WorldState->SetState(TEXT("scene"), TEXT("current"), SceneTemplate.SceneId.ToString());
        }
    }

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
