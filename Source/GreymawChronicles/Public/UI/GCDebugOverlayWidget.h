#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Gameplay/AbilityCheckResolver.h"
#include "GCDebugOverlayWidget.generated.h"

class UTextBlock;
class UVerticalBox;
class UDMBrainSubsystem;

/**
 * Sprint G: On-screen debug feedback panel.
 * Shows DM state, last action count, and last check result.
 */
UCLASS()
class GREYMAWCHRONICLES_API UGCDebugOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void SetDMState(bool bIsProcessing);

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void SetLastActionCount(int32 Count);

	UFUNCTION(BlueprintCallable, Category = "Debug")
	void SetLastCheckResult(const FAbilityCheckResult& Result);

protected:
	virtual void NativeConstruct() override;

private:
	void EnsureFallbackWidgets();
	void BindToDMBrain();

	UFUNCTION()
	void HandleProcessingStateChanged(bool bIsProcessing);

	UFUNCTION()
	void HandleActionsReady(const TArray<FDMAction>& Actions);

	UFUNCTION()
	void HandleDiceResolved(const FAbilityCheckResult& Result);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DMStateText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ActionCountText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CheckResultText;

	TWeakObjectPtr<UDMBrainSubsystem> DMBrain;
};
