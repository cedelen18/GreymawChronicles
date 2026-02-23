#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GCSaveLoadWidget.generated.h"

class UVerticalBox;
class UTextBlock;

/**
 * Sprint L: Multi-slot save/load widget.
 * Shows 3 manual slots + auto-save status.
 */
UCLASS()
class GREYMAWCHRONICLES_API UGCSaveLoadWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "UI|Save")
    void ToggleVisibility();

    UFUNCTION(BlueprintCallable, Category = "UI|Save")
    void RefreshSlots();

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UVerticalBox> SlotListBox;
};
