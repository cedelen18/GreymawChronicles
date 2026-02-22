#include "Core/GCGameMode.h"

#include "UI/GCMainHUDWidget.h"
#include "Blueprint/UserWidget.h"

AGCGameMode::AGCGameMode()
{
    MainHUDWidgetClass = UGCMainHUDWidget::StaticClass();
}

void AGCGameMode::BeginPlay()
{
    Super::BeginPlay();

    if (MainHUDWidgetClass)
    {
        MainHUDWidget = CreateWidget<UGCMainHUDWidget>(GetWorld(), MainHUDWidgetClass);
        if (MainHUDWidget)
        {
            MainHUDWidget->AddToViewport();
        }
    }
}
