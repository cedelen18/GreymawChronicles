#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DMTextPanelWidget.generated.h"

class UScrollBox;
class UTextBlock;

UCLASS()
class GREYMAWCHRONICLES_API UDMTextPanelWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "DM")
    void PushNarration(const FString& NarrationText);

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UScrollBox> NarrationScrollBox;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> NarrationTextBlock;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DM")
    float TypewriterCharsPerSecond = 30.0f;

private:
    FString FullText;
    int32 TypewriterIndex = 0;
    FTimerHandle TypewriterTimer;

    void EnsureFallbackWidgets();
    void AdvanceTypewriter();
};
