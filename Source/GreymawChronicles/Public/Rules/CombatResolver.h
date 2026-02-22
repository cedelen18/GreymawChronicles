#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Gameplay/GCCharacterSheet.h"
#include "Gameplay/DiceRoller.h"
#include "CombatResolver.generated.h"

USTRUCT(BlueprintType)
struct FAttackResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Roll = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Modifier = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Total = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 TargetAC = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bHit = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bCriticalHit = false;
};

USTRUCT(BlueprintType)
struct FWeaponAttackData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DamageDiceCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DamageDieSize = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EGCAbility AttackAbility = EGCAbility::Strength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bProficient = true;
};

UCLASS(BlueprintType)
class GREYMAWCHRONICLES_API UCombatResolver : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
    TObjectPtr<UDiceRoller> DiceRoller;

    UFUNCTION(BlueprintCallable, Category = "Rules")
    void Initialize(UDiceRoller* InDiceRoller);

    UFUNCTION(BlueprintCallable, Category = "Rules")
    FAttackResult ResolveAttackRoll(const UGCCharacterSheet* Attacker, const FWeaponAttackData& Weapon, const UGCCharacterSheet* Defender, bool bAdvantage, bool bDisadvantage);

    UFUNCTION(BlueprintCallable, Category = "Rules")
    int32 CalculateDamage(const FWeaponAttackData& Weapon, int32 Modifier, bool bCritical);

    UFUNCTION(BlueprintCallable, Category = "Rules")
    int32 ApplyDamage(UGCCharacterSheet* Target, int32 Damage);

    UFUNCTION(BlueprintCallable, Category = "Rules")
    TArray<UGCCharacterSheet*> ResolveInitiative(const TArray<UGCCharacterSheet*>& Combatants);

private:
    EGCRollMode ResolveRollMode(bool bAdvantage, bool bDisadvantage) const;
    UDiceRoller* GetDiceRoller();
};
