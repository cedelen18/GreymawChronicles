#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DiceRoller.generated.h"

UENUM(BlueprintType)
enum class EGCRollMode : uint8
{
    Normal,
    Advantage,
    Disadvantage
};

USTRUCT(BlueprintType)
struct FDiceRollResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString Label;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 NumDice = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 DieSize = 20;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<int32> IndividualRolls;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 PrimaryRoll = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 SecondaryRoll = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Total = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EGCRollMode Mode = EGCRollMode::Normal;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiceRolled, const FDiceRollResult&, Result);

UCLASS(BlueprintType)
class GREYMAWCHRONICLES_API UDiceRoller : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Dice")
    FOnDiceRolled OnDiceRolled;

    UFUNCTION(BlueprintCallable, Category = "Dice")
    int32 RollD20();

    UFUNCTION(BlueprintCallable, Category = "Dice")
    int32 RollDice(int32 NumDice, int32 DieSize);

    UFUNCTION(BlueprintCallable, Category = "Dice")
    int32 RollD20WithAdvantage();

    UFUNCTION(BlueprintCallable, Category = "Dice")
    int32 RollD20WithDisadvantage();

    UFUNCTION(BlueprintCallable, Category = "Dice")
    FDiceRollResult RollDiceDetailed(int32 NumDice, int32 DieSize, EGCRollMode Mode = EGCRollMode::Normal, const FString& Label = TEXT(""));
};
