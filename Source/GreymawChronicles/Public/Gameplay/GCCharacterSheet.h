#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GCCharacterSheet.generated.h"

UENUM(BlueprintType)
enum class EGCRace : uint8
{
    Human UMETA(DisplayName = "Human"),
    Elf UMETA(DisplayName = "Elf"),
    Dwarf UMETA(DisplayName = "Dwarf"),
    Unknown UMETA(DisplayName = "Unknown")
};

UENUM(BlueprintType)
enum class EGCClass : uint8
{
    Fighter UMETA(DisplayName = "Fighter"),
    Rogue UMETA(DisplayName = "Rogue"),
    Wizard UMETA(DisplayName = "Wizard"),
    Cleric UMETA(DisplayName = "Cleric"),
    Companion UMETA(DisplayName = "Companion"),
    Unknown UMETA(DisplayName = "Unknown")
};

UENUM(BlueprintType)
enum class EGCAbility : uint8
{
    Strength UMETA(DisplayName = "STR"),
    Dexterity UMETA(DisplayName = "DEX"),
    Constitution UMETA(DisplayName = "CON"),
    Intelligence UMETA(DisplayName = "INT"),
    Wisdom UMETA(DisplayName = "WIS"),
    Charisma UMETA(DisplayName = "CHA")
};

UENUM(BlueprintType)
enum class EGCSkill : uint8
{
    Acrobatics,
    AnimalHandling,
    Arcana,
    Athletics,
    Deception,
    History,
    Insight,
    Intimidation,
    Investigation,
    Medicine,
    Nature,
    Perception,
    Performance,
    Persuasion,
    Religion,
    SleightOfHand,
    Stealth,
    Survival
};

USTRUCT(BlueprintType)
struct FGCSpellSlotState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Max = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Current = 0;
};

UCLASS(BlueprintType)
class GREYMAWCHRONICLES_API UGCCharacterSheet : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    EGCRace Race = EGCRace::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    EGCClass CharacterClass = EGCClass::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    int32 Level = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals")
    int32 MaxHP = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals")
    int32 CurrentHP = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vitals")
    int32 ArmorClass = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
    int32 ProficiencyBonus = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
    TMap<EGCAbility, int32> AbilityScores;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
    TSet<EGCAbility> SavingThrowProficiencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rules")
    TSet<EGCSkill> SkillProficiencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
    TArray<FString> ClassFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Features")
    TArray<FString> Equipment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spells")
    TMap<int32, FGCSpellSlotState> SpellSlotsByLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spells")
    TArray<FString> Spells;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spells")
    TArray<FString> Cantrips;

    UFUNCTION(BlueprintCallable, Category = "Rules")
    int32 GetAbilityModifier(EGCAbility Ability) const;

    UFUNCTION(BlueprintCallable, Category = "Rules")
    int32 GetSkillModifier(EGCSkill Skill) const;

    UFUNCTION(BlueprintCallable, Category = "Rules")
    int32 GetSavingThrowModifier(EGCAbility Ability) const;

    UFUNCTION(BlueprintCallable, Category = "Serialization")
    FString ToCompactJSON() const;

    UFUNCTION(BlueprintCallable, Category = "Serialization")
    bool FromJSON(const FString& InJSON, FString& OutError);

    static EGCAbility SkillToAbility(EGCSkill Skill);
};
