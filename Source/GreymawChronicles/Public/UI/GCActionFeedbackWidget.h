#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/AbilityCheckResolver.h"
#include "GCActionFeedbackWidget.generated.h"

class UTextBlock;
class UVerticalBox;
class UDMBrainSubsystem;
class UActionDirectorSubsystem;

/**
 * Sprint H: Transient action-feedback toasts that appear mid-screen.
 * Shows movement targets, check requests, check results, and world-state updates.
 * Each toast auto-fades after a configurable duration.
 */
UCLASS()
class GREYMAWCHRONICLES_API UGCActionFeedbackWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Push a single-line toast that auto-fades. */
    UFUNCTION(BlueprintCallable, Category = "UI|Feedback")
    void PushToast(const FString& Message, const FLinearColor& Color = FLinearColor(1.f, 1.f, 1.f, 1.f));

    /** Push a toast with custom duration (Sprint I: for check results). */
    void PushToastWithDuration(const FString& Message, const FLinearColor& Color, float Duration);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    void EnsureFallbackWidgets();
    void BindToSubsystems();

    UFUNCTION()
    void HandleProcessingStateChanged(bool bIsProcessing);

    UFUNCTION()
    void HandleActionsReady(const TArray<FDMAction>& Actions);

    UFUNCTION()
    void HandleDiceResolved(const FAbilityCheckResult& Result);

    UFUNCTION()
    void HandleActionSequenceComplete();

    struct FToastEntry
    {
        FString Text;
        FLinearColor Color;
        float RemainingSeconds;
    };

    TArray<FToastEntry> ActiveToasts;

    UPROPERTY()
    TObjectPtr<UVerticalBox> ToastContainer;

    TWeakObjectPtr<UDMBrainSubsystem> DMBrain;
    TWeakObjectPtr<UActionDirectorSubsystem> ActionDirector;

    /** How long each toast stays visible. */
    float ToastDuration = 3.0f;

    /** Sprint I: Longer duration for check result toasts. */
    float CheckToastDuration = 5.0f;

    /** Maximum simultaneous toasts (oldest removed first). Sprint I: 4 -> 6. */
    int32 MaxToasts = 6;
};
