#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GCHUDOverlayWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class GREYMAWCHRONICLES_API UGCHUDOverlayWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetPlayerStatus(const FString& Name, int32 Level, float CurrentHP, float MaxHP);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void SetCompanionStatus(const FString& Name, int32 Level, float CurrentHP, float MaxHP);

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> PlayerText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> PlayerHPBar;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> CompanionText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> CompanionHPBar;

private:
    void EnsureFallbackWidgets();
    static FLinearColor GetHPColor(float Ratio);
};
