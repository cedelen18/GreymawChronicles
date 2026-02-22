#include "UI/GCPlayerInputWidget.h"

#include "DungeonMaster/DMBrainSubsystem.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Blueprint/WidgetTree.h"

void UGCPlayerInputWidget::NativeConstruct()
{
    Super::NativeConstruct();
    EnsureFallbackWidgets();

    if (InputTextBox)
    {
        InputTextBox->SetHintText(FText::FromString(TEXT("What do you do?")));
        InputTextBox->OnTextCommitted.AddDynamic(this, &UGCPlayerInputWidget::HandleInputCommitted);
    }

    if (SubmitButton)
    {
        SubmitButton->OnClicked.AddDynamic(this, &UGCPlayerInputWidget::HandleSubmitClicked);
    }

    if (UGameInstance* GI = GetGameInstance())
    {
        DMBrain = GI->GetSubsystem<UDMBrainSubsystem>();
        if (DMBrain.IsValid())
        {
            DMBrain->OnDMProcessingStateChanged.AddDynamic(this, &UGCPlayerInputWidget::HandleProcessingStateChanged);
            HandleProcessingStateChanged(DMBrain->IsProcessing());
        }
    }
}

void UGCPlayerInputWidget::HandleInputCommitted(const FText& Text, ETextCommit::Type CommitType)
{
    if (CommitType == ETextCommit::OnEnter)
    {
        SubmitCurrentText();
    }
}

void UGCPlayerInputWidget::HandleSubmitClicked()
{
    SubmitCurrentText();
}

void UGCPlayerInputWidget::HandleProcessingStateChanged(bool bIsProcessing)
{
    if (InputTextBox)
    {
        InputTextBox->SetIsEnabled(!bIsProcessing);
    }

    if (SubmitButton)
    {
        SubmitButton->SetIsEnabled(!bIsProcessing);
    }

    if (WaitingText)
    {
        WaitingText->SetText(FText::FromString(bIsProcessing ? TEXT("...") : TEXT("")));
    }
}

void UGCPlayerInputWidget::SubmitCurrentText()
{
    if (!InputTextBox || !DMBrain.IsValid() || DMBrain->IsProcessing())
    {
        return;
    }

    const FString TextToSend = InputTextBox->GetText().ToString().TrimStartAndEnd();
    if (TextToSend.IsEmpty())
    {
        return;
    }

    InputTextBox->SetText(FText::GetEmpty());
    DMBrain->ProcessPlayerInput(TextToSend);
}

void UGCPlayerInputWidget::EnsureFallbackWidgets()
{
    if (InputTextBox && SubmitButton && WaitingText)
    {
        return;
    }

    UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InputRoot"));
    WidgetTree->RootWidget = Root;

    InputTextBox = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("InputTextBox"));
    SubmitButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("SubmitButton"));
    WaitingText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WaitingText"));

    Root->AddChildToVerticalBox(InputTextBox);
    Root->AddChildToVerticalBox(SubmitButton);
    Root->AddChildToVerticalBox(WaitingText);
}
