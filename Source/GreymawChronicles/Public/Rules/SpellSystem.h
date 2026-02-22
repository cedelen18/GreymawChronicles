#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Gameplay/GCCharacterSheet.h"
#include "Gameplay/DiceRoller.h"
#include "SpellSystem.generated.h"

USTRUCT(BlueprintType)
struct FSpellAttackResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Roll = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Modifier = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 Total = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bHit = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bCriticalHit = false;
};

USTRUCT(BlueprintType)
struct FSpellDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FString SpellId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 SpellLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bRequiresAttackRoll = false;
};

UCLASS(BlueprintType)
class GREYMAWCHRONICLES_API USpellSystem : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
    TObjectPtr<UDiceRoller> DiceRoller;

    UFUNCTION(BlueprintCallable, Category = "Rules")
    void Initialize(UDiceRoller* InDiceRoller);

    UFUNCTION(BlueprintCallable, Category = "Rules")
    bool HasSpellSlot(const UGCCharacterSheet* Caster, int32 SpellLevel) const;

    UFUNCTION(BlueprintCallable, Category = "Rules")
    bool ExpendSpellSlot(UGCCharacterSheet* Caster, int32 SpellLevel);

    UFUNCTION(BlueprintCallable, Category = "Rules")
    int32 GetSpellSaveDC(const UGCCharacterSheet* Caster) const;

    UFUNCTION(BlueprintCallable, Category = "Rules")
    int32 GetSpellAttackModifier(const UGCCharacterSheet* Caster) const;

    UFUNCTION(BlueprintCallable, Category = "Rules")
    FSpellAttackResult ResolveSpellAttack(const UGCCharacterSheet* Caster, int32 TargetArmorClass, bool bAdvantage, bool bDisadvantage);

    UFUNCTION(BlueprintCallable, Category = "Rules")
    bool TryGetSpellDefinition(const FString& SpellId, FSpellDefinition& OutSpell) const;

private:
    void BuildMVPDefinitions();
    EGCAbility ResolveSpellcastingAbility(const UGCCharacterSheet* Caster) const;
    EGCRollMode ResolveRollMode(bool bAdvantage, bool bDisadvantage) const;
    UDiceRoller* GetDiceRoller();

    UPROPERTY()
    TMap<FString, FSpellDefinition> Definitions;
};
