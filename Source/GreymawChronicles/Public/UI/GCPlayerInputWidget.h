#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GCPlayerInputWidget.generated.h"

class UButton;
class UEditableTextBox;
class UTextBlock;
class UDMBrainSubsystem;

UCLASS()
class GREYMAWCHRONICLES_API UGCPlayerInputWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UEditableTextBox> InputTextBox;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> SubmitButton;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> WaitingText;

private:
    TWeakObjectPtr<UDMBrainSubsystem> DMBrain;

    UFUNCTION()
    void HandleInputCommitted(const FText& Text, ETextCommit::Type CommitType);

    UFUNCTION()
    void HandleSubmitClicked();

    UFUNCTION()
    void HandleProcessingStateChanged(bool bIsProcessing);

    void SubmitCurrentText();
    void EnsureFallbackWidgets();
};
