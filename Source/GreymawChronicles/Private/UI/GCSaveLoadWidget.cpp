#include "UI/GCSaveLoadWidget.h"
#include "Core/GCGameInstance.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogSaveLoadWidget, Log, All);

void UGCSaveLoadWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetVisibility(ESlateVisibility::Collapsed);
}

void UGCSaveLoadWidget::ToggleVisibility()
{
    if (GetVisibility() == ESlateVisibility::Visible)
    {
        SetVisibility(ESlateVisibility::Collapsed);
    }
    else
    {
        RefreshSlots();
        SetVisibility(ESlateVisibility::Visible);
    }
}

void UGCSaveLoadWidget::RefreshSlots()
{
    if (!SlotListBox)
    {
        UE_LOG(LogSaveLoadWidget, Warning, TEXT("SlotListBox not bound."));
        return;
    }

    SlotListBox->ClearChildren();

    // Show 3 manual slots + auto-save status
    for (int32 i = 0; i < 3; ++i)
    {
        const FString SlotName = FString::Printf(TEXT("GreymawSlot_%d"), i);
        const bool bExists = UGameplayStatics::DoesSaveGameExist(SlotName, 0);

        UTextBlock* SlotText = NewObject<UTextBlock>(this);
        if (!SlotText) continue;

        const FString DisplayText = bExists
            ? FString::Printf(TEXT("Slot %d: [Saved]"), i + 1)
            : FString::Printf(TEXT("Slot %d: [Empty]"), i + 1);

        SlotText->SetText(FText::FromString(DisplayText));
        SlotListBox->AddChild(SlotText);
    }

    // Auto-save status
    {
        const bool bAutoExists = UGameplayStatics::DoesSaveGameExist(TEXT("GreymawAutoSave"), 0);
        UTextBlock* AutoText = NewObject<UTextBlock>(this);
        if (AutoText)
        {
            AutoText->SetText(FText::FromString(bAutoExists ? TEXT("Auto-save: [Available]") : TEXT("Auto-save: [None]")));
            SlotListBox->AddChild(AutoText);
        }
    }
}
