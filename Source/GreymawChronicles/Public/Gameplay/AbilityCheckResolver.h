#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Gameplay/GCCharacterSheet.h"
#include "Gameplay/DiceRoller.h"
#include "AbilityCheckResolver.generated.h"

USTRUCT(BlueprintType)
struct FAbilityCheckResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Roll = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Modifier = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Total = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 DC = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bSuccess = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bCriticalSuccess = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bCriticalFailure = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EGCRollMode RollMode = EGCRollMode::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString CheckType;
};

UCLASS(BlueprintType)
class GREYMAWCHRONICLES_API UAbilityCheckResolver : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
    TObjectPtr<UDiceRoller> DiceRoller;

    UFUNCTION(BlueprintCallable, Category = "Rules")
    void Initialize(UDiceRoller* InDiceRoller);

    UFUNCTION(BlueprintCallable, Category = "Rules")
    FAbilityCheckResult ResolveAbilityCheck(const UGCCharacterSheet* Sheet, EGCSkill Skill, int32 DC, bool bAdvantage, bool bDisadvantage);

    UFUNCTION(BlueprintCallable, Category = "Rules")
    FAbilityCheckResult ResolveSavingThrow(const UGCCharacterSheet* Sheet, EGCAbility Ability, int32 DC, bool bAdvantage, bool bDisadvantage);

private:
    EGCRollMode ResolveRollMode(bool bAdvantage, bool bDisadvantage) const;
    int32 RollD20ForMode(EGCRollMode Mode) const;
};
