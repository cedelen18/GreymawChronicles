#include "UI/GCDebugOverlayWidget.h"

#include "DungeonMaster/DMBrainSubsystem.h"
#include "DungeonMaster/DMTypes.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

void UGCDebugOverlayWidget::NativeConstruct()
{
    Super::NativeConstruct();
    EnsureFallbackWidgets();
    BindToDMBrain();

    // Defaults
    SetDMState(false);
    SetLastActionCount(0);

    if (CheckResultText)
    {
        CheckResultText->SetText(FText::FromString(TEXT("[Check] —")));
    }
}

void UGCDebugOverlayWidget::EnsureFallbackWidgets()
{
    if (DMStateText && ActionCountText && CheckResultText)
    {
        return;
    }

    if (!WidgetTree)
    {
        UE_LOG(LogTemp, Warning, TEXT("GCDebugOverlayWidget: WidgetTree is null, cannot create fallback widgets."));
        return;
    }

    UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("DebugRoot"));
    WidgetTree->RootWidget = Root;

    auto MakeText = [&](const FString& Name) -> UTextBlock*
    {
        UTextBlock* Block = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), *Name);
        if (Block)
        {
            FSlateFontInfo Font = Block->GetFont();
            Font.Size = 14;
            Block->SetFont(Font);
            Block->SetColorAndOpacity(FSlateColor(FLinearColor(0.0f, 1.0f, 0.4f, 1.0f)));
            Root->AddChildToVerticalBox(Block);
        }
        return Block;
    };

    DMStateText = MakeText(TEXT("DMStateText"));
    ActionCountText = MakeText(TEXT("ActionCountText"));
    CheckResultText = MakeText(TEXT("CheckResultText"));
}

void UGCDebugOverlayWidget::BindToDMBrain()
{
    if (UGameInstance* GI = GetGameInstance())
    {
        DMBrain = GI->GetSubsystem<UDMBrainSubsystem>();
        if (DMBrain.IsValid())
        {
            DMBrain->OnDMProcessingStateChanged.AddDynamic(this, &UGCDebugOverlayWidget::HandleProcessingStateChanged);
            DMBrain->OnDMActionsReady.AddDynamic(this, &UGCDebugOverlayWidget::HandleActionsReady);
            DMBrain->OnDMDiceResolved.AddDynamic(this, &UGCDebugOverlayWidget::HandleDiceResolved);
        }
    }
}

void UGCDebugOverlayWidget::SetDMState(bool bIsProcessing)
{
    if (DMStateText)
    {
        const FString State = bIsProcessing ? TEXT("Processing...") : TEXT("Idle");
        DMStateText->SetText(FText::FromString(FString::Printf(TEXT("[DM] %s"), *State)));
        DMStateText->SetColorAndOpacity(FSlateColor(bIsProcessing
            ? FLinearColor(1.0f, 0.8f, 0.0f, 1.0f)
            : FLinearColor(0.0f, 1.0f, 0.4f, 1.0f)));
    }
}

void UGCDebugOverlayWidget::SetLastActionCount(int32 Count)
{
    if (ActionCountText)
    {
        ActionCountText->SetText(FText::FromString(FString::Printf(TEXT("[Actions] %d queued"), Count)));
    }
}

void UGCDebugOverlayWidget::SetLastCheckResult(const FAbilityCheckResult& Result)
{
    if (CheckResultText)
    {
        const FString CritTag = Result.bCriticalSuccess ? TEXT(" CRIT!") : (Result.bCriticalFailure ? TEXT(" FUMBLE!") : TEXT(""));
        const FString PassFail = Result.bSuccess ? TEXT("PASS") : TEXT("FAIL");
        CheckResultText->SetText(FText::FromString(FString::Printf(
            TEXT("[Check] %s | Roll:%d + Mod:%d = %d vs DC %d  %s%s"),
            *Result.CheckType,
            Result.Roll,
            Result.Modifier,
            Result.Total,
            Result.DC,
            *PassFail,
            *CritTag)));
        CheckResultText->SetColorAndOpacity(FSlateColor(Result.bSuccess
            ? FLinearColor(0.0f, 1.0f, 0.4f, 1.0f)
            : FLinearColor(1.0f, 0.2f, 0.2f, 1.0f)));
    }
}

void UGCDebugOverlayWidget::HandleProcessingStateChanged(bool bIsProcessing)
{
    SetDMState(bIsProcessing);
}

void UGCDebugOverlayWidget::HandleActionsReady(const TArray<FDMAction>& Actions)
{
    SetLastActionCount(Actions.Num());
}

void UGCDebugOverlayWidget::HandleDiceResolved(const FAbilityCheckResult& Result)
{
    SetLastCheckResult(Result);
}
