#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GCGameMode.generated.h"

class UGCMainHUDWidget;

UCLASS()
class GREYMAWCHRONICLES_API AGCGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AGCGameMode();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UGCMainHUDWidget> MainHUDWidgetClass;

    UPROPERTY()
    TObjectPtr<UGCMainHUDWidget> MainHUDWidget;
};
