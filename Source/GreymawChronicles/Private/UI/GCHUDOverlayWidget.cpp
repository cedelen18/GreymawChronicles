#include "UI/GCHUDOverlayWidget.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"

void UGCHUDOverlayWidget::NativeConstruct()
{
    Super::NativeConstruct();
    EnsureFallbackWidgets();

    SetPlayerStatus(TEXT("Player"), 3, 24.0f, 24.0f);
    SetCompanionStatus(TEXT("Kael"), 3, 28.0f, 28.0f);
}

void UGCHUDOverlayWidget::SetPlayerStatus(const FString& Name, int32 Level, float CurrentHP, float MaxHP)
{
    const float Ratio = MaxHP > 0.f ? FMath::Clamp(CurrentHP / MaxHP, 0.f, 1.f) : 0.f;

    if (PlayerText)
    {
        PlayerText->SetText(FText::FromString(FString::Printf(TEXT("%s Lv.%d  %.0f/%.0f HP"), *Name, Level, CurrentHP, MaxHP)));
    }
    if (PlayerHPBar)
    {
        PlayerHPBar->SetPercent(Ratio);
        PlayerHPBar->SetFillColorAndOpacity(GetHPColor(Ratio));
    }
}

void UGCHUDOverlayWidget::SetCompanionStatus(const FString& Name, int32 Level, float CurrentHP, float MaxHP)
{
    const float Ratio = MaxHP > 0.f ? FMath::Clamp(CurrentHP / MaxHP, 0.f, 1.f) : 0.f;

    if (CompanionText)
    {
        CompanionText->SetText(FText::FromString(FString::Printf(TEXT("%s Lv.%d  %.0f/%.0f HP"), *Name, Level, CurrentHP, MaxHP)));
    }
    if (CompanionHPBar)
    {
        CompanionHPBar->SetPercent(Ratio);
        CompanionHPBar->SetFillColorAndOpacity(GetHPColor(Ratio));
    }
}

void UGCHUDOverlayWidget::EnsureFallbackWidgets()
{
    if (PlayerText && PlayerHPBar && CompanionText && CompanionHPBar)
    {
        return;
    }

    UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("HUDRoot"));
    WidgetTree->RootWidget = Root;

    PlayerText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("PlayerText"));
    PlayerHPBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("PlayerHPBar"));
    CompanionText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CompanionText"));
    CompanionHPBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("CompanionHPBar"));

    Root->AddChildToVerticalBox(PlayerText);
    Root->AddChildToVerticalBox(PlayerHPBar);
    Root->AddChildToVerticalBox(CompanionText);
    Root->AddChildToVerticalBox(CompanionHPBar);
}

FLinearColor UGCHUDOverlayWidget::GetHPColor(float Ratio)
{
    if (Ratio > 0.6f)
    {
        return FLinearColor(0.1f, 0.75f, 0.2f, 1.0f);
    }
    if (Ratio > 0.3f)
    {
        return FLinearColor(0.9f, 0.75f, 0.1f, 1.0f);
    }
    return FLinearColor(0.85f, 0.1f, 0.1f, 1.0f);
}
