#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GCGameMode.generated.h"

class UGCMainHUDWidget;
struct FTimerHandle;

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

private:
    void EnsureHUDBootstrap();

    UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (ClampMin = "0.05", ClampMax = "2.0"))
    float HUDSpawnRetryDelaySeconds = 0.2f;

    UPROPERTY(EditDefaultsOnly, Category = "UI", meta = (ClampMin = "1", ClampMax = "50"))
    int32 MaxHUDSpawnRetries = 10;

    int32 HUDSpawnAttempts = 0;
    FTimerHandle HUDSpawnRetryHandle;
};
