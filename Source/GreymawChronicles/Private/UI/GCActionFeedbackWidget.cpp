#include "UI/GCActionFeedbackWidget.h"

#include "ActionSystem/ActionDirectorSubsystem.h"
#include "DungeonMaster/DMBrainSubsystem.h"
#include "DungeonMaster/DMTypes.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

void UGCActionFeedbackWidget::NativeConstruct()
{
    Super::NativeConstruct();
    EnsureFallbackWidgets();
    BindToSubsystems();
}

void UGCActionFeedbackWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (ActiveToasts.Num() == 0)
    {
        return;
    }

    // Tick down toast lifetimes
    bool bNeedsRebuild = false;
    for (int32 i = ActiveToasts.Num() - 1; i >= 0; --i)
    {
        ActiveToasts[i].RemainingSeconds -= InDeltaTime;
        if (ActiveToasts[i].RemainingSeconds <= 0.0f)
        {
            ActiveToasts.RemoveAt(i);
            bNeedsRebuild = true;
        }
    }

    if (bNeedsRebuild && ToastContainer)
    {
        ToastContainer->ClearChildren();
        for (const FToastEntry& Toast : ActiveToasts)
        {
            UTextBlock* Block = NewObject<UTextBlock>(ToastContainer);
            if (Block)
            {
                Block->SetText(FText::FromString(Toast.Text));

                // Fade alpha as toast expires
                const float Alpha = FMath::Clamp(Toast.RemainingSeconds / 0.5f, 0.0f, 1.0f);
                FLinearColor FadedColor = Toast.Color;
                FadedColor.A = Alpha;
                Block->SetColorAndOpacity(FSlateColor(FadedColor));

                FSlateFontInfo Font = Block->GetFont();
                Font.Size = 16;
                Block->SetFont(Font);

                ToastContainer->AddChildToVerticalBox(Block);
            }
        }
    }
}

void UGCActionFeedbackWidget::PushToast(const FString& Message, const FLinearColor& Color)
{
    // Evict oldest if at capacity
    while (ActiveToasts.Num() >= MaxToasts)
    {
        ActiveToasts.RemoveAt(0);
    }

    FToastEntry Entry;
    Entry.Text = Message;
    Entry.Color = Color;
    Entry.RemainingSeconds = ToastDuration;
    ActiveToasts.Add(Entry);

    // Immediately render
    if (ToastContainer)
    {
        UTextBlock* Block = NewObject<UTextBlock>(ToastContainer);
        if (Block)
        {
            Block->SetText(FText::FromString(Message));
            Block->SetColorAndOpacity(FSlateColor(Color));

            FSlateFontInfo Font = Block->GetFont();
            Font.Size = 16;
            Block->SetFont(Font);

            ToastContainer->AddChildToVerticalBox(Block);
        }
    }
}

void UGCActionFeedbackWidget::EnsureFallbackWidgets()
{
    if (ToastContainer)
    {
        return;
    }

    ToastContainer = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ToastContainer"));
    WidgetTree->RootWidget = ToastContainer;
}

void UGCActionFeedbackWidget::BindToSubsystems()
{
    if (UGameInstance* GI = GetGameInstance())
    {
        DMBrain = GI->GetSubsystem<UDMBrainSubsystem>();
        if (DMBrain.IsValid())
        {
            DMBrain->OnDMProcessingStateChanged.AddDynamic(this, &UGCActionFeedbackWidget::HandleProcessingStateChanged);
            DMBrain->OnDMActionsReady.AddDynamic(this, &UGCActionFeedbackWidget::HandleActionsReady);
            DMBrain->OnDMDiceResolved.AddDynamic(this, &UGCActionFeedbackWidget::HandleDiceResolved);
        }

        ActionDirector = GI->GetSubsystem<UActionDirectorSubsystem>();
        if (ActionDirector.IsValid())
        {
            ActionDirector->OnActionSequenceComplete.AddDynamic(this, &UGCActionFeedbackWidget::HandleActionSequenceComplete);
        }
    }
}

void UGCActionFeedbackWidget::HandleProcessingStateChanged(bool bIsProcessing)
{
    if (bIsProcessing)
    {
        PushToast(TEXT("DM is resolving..."), FLinearColor(0.7f, 0.7f, 0.7f, 1.0f));
    }
}

void UGCActionFeedbackWidget::HandleActionsReady(const TArray<FDMAction>& Actions)
{
    for (const FDMAction& Action : Actions)
    {
        if (Action.bHasMoveTarget)
        {
            PushToast(FString::Printf(TEXT("> Moving %s to target"), *Action.Actor),
                FLinearColor(0.4f, 0.8f, 1.0f, 1.0f));
        }
    }
}

void UGCActionFeedbackWidget::HandleDiceResolved(const FAbilityCheckResult& Result)
{
    const FString CritTag = Result.bCriticalSuccess ? TEXT(" CRITICAL!") : (Result.bCriticalFailure ? TEXT(" FUMBLE!") : TEXT(""));
    const FString PassFail = Result.bSuccess ? TEXT("PASSED") : TEXT("FAILED");

    const FLinearColor Color = Result.bSuccess
        ? FLinearColor(0.1f, 1.0f, 0.3f, 1.0f)
        : FLinearColor(1.0f, 0.3f, 0.2f, 1.0f);

    PushToast(FString::Printf(TEXT("> %s check: %d vs DC %d -- %s%s"),
        *Result.CheckType, Result.Total, Result.DC, *PassFail, *CritTag), Color);
}

void UGCActionFeedbackWidget::HandleActionSequenceComplete()
{
    PushToast(TEXT("> Ready for input"), FLinearColor(0.6f, 0.6f, 0.6f, 1.0f));
}
