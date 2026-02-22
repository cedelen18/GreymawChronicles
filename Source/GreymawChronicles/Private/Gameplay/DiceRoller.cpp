#include "Gameplay/DiceRoller.h"

int32 UDiceRoller::RollD20()
{
    return RollDiceDetailed(1, 20, EGCRollMode::Normal, TEXT("d20")).Total;
}

int32 UDiceRoller::RollDice(int32 NumDice, int32 DieSize)
{
    return RollDiceDetailed(NumDice, DieSize, EGCRollMode::Normal, FString::Printf(TEXT("%dd%d"), NumDice, DieSize)).Total;
}

int32 UDiceRoller::RollD20WithAdvantage()
{
    return RollDiceDetailed(1, 20, EGCRollMode::Advantage, TEXT("d20(adv)")).Total;
}

int32 UDiceRoller::RollD20WithDisadvantage()
{
    return RollDiceDetailed(1, 20, EGCRollMode::Disadvantage, TEXT("d20(dis)")).Total;
}

FDiceRollResult UDiceRoller::RollDiceDetailed(int32 NumDice, int32 DieSize, EGCRollMode Mode, const FString& Label)
{
    FDiceRollResult Result;
    Result.Label = Label;
    Result.NumDice = FMath::Max(1, NumDice);
    Result.DieSize = FMath::Max(2, DieSize);
    Result.Mode = Mode;

    if (Result.NumDice == 1 && Result.DieSize == 20 && Mode != EGCRollMode::Normal)
    {
        Result.PrimaryRoll = FMath::RandRange(1, 20);
        Result.SecondaryRoll = FMath::RandRange(1, 20);
        Result.IndividualRolls = {Result.PrimaryRoll, Result.SecondaryRoll};
        Result.Total = (Mode == EGCRollMode::Advantage) ? FMath::Max(Result.PrimaryRoll, Result.SecondaryRoll) : FMath::Min(Result.PrimaryRoll, Result.SecondaryRoll);
    }
    else
    {
        for (int32 Index = 0; Index < Result.NumDice; ++Index)
        {
            const int32 Roll = FMath::RandRange(1, Result.DieSize);
            Result.IndividualRolls.Add(Roll);
            Result.Total += Roll;
        }

        if (Result.IndividualRolls.Num() > 0)
        {
            Result.PrimaryRoll = Result.IndividualRolls[0];
        }
    }

    OnDiceRolled.Broadcast(Result);
    return Result;
}
