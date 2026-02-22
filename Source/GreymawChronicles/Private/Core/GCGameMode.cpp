#include "Core/GCGameMode.h"

#include "UI/GCMainHUDWidget.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AGCGameMode::AGCGameMode()
{
    MainHUDWidgetClass = UGCMainHUDWidget::StaticClass();
}

void AGCGameMode::BeginPlay()
{
    Super::BeginPlay();
    EnsureHUDBootstrap();
}

void AGCGameMode::EnsureHUDBootstrap()
{
    ++HUDSpawnAttempts;

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (!PlayerController)
    {
        if (HUDSpawnAttempts < MaxHUDSpawnRetries)
        {
            GetWorldTimerManager().SetTimer(HUDSpawnRetryHandle, this, &AGCGameMode::EnsureHUDBootstrap, HUDSpawnRetryDelaySeconds, false);
        }
        return;
    }

    if (!MainHUDWidgetClass)
    {
        return;
    }

    if (!MainHUDWidget)
    {
        TArray<UUserWidget*> ExistingWidgets;
        UWidgetBlueprintLibrary::GetAllWidgetsOfClass(this, ExistingWidgets, MainHUDWidgetClass, false);
        if (ExistingWidgets.Num() > 0)
        {
            MainHUDWidget = Cast<UGCMainHUDWidget>(ExistingWidgets[0]);
        }
    }

    if (!MainHUDWidget)
    {
        MainHUDWidget = CreateWidget<UGCMainHUDWidget>(PlayerController, MainHUDWidgetClass);
        if (MainHUDWidget)
        {
            MainHUDWidget->AddToViewport(0);
        }
    }

    if (MainHUDWidget)
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        InputMode.SetHideCursorDuringCapture(false);
        PlayerController->SetInputMode(InputMode);
        PlayerController->SetShowMouseCursor(true);
    }
}
