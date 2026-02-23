#pragma once

#include "CoreMinimal.h"
#include "Rules/CombatResolver.h"
#include "GCEnemyTemplate.generated.h"

/**
 * Sprint L: Enemy stat block for combat encounters.
 * Mirrors key fields from UGCCharacterSheet but as a lightweight struct
 * suitable for spawning ephemeral enemies without full DataAsset overhead.
 */
USTRUCT(BlueprintType)
struct GREYMAWCHRONICLES_API FGCEnemyTemplate
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    FString CreatureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    int32 MaxHP = 7;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    int32 CurrentHP = 7;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    int32 ArmorClass = 15;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    int32 AttackModifier = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    FWeaponAttackData Weapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    int32 InitiativeModifier = 2;

    /** Helper: create a standard goblin. */
    static FGCEnemyTemplate MakeGoblin()
    {
        FGCEnemyTemplate Goblin;
        Goblin.CreatureName = TEXT("Goblin");
        Goblin.MaxHP = 7;
        Goblin.CurrentHP = 7;
        Goblin.ArmorClass = 15;
        Goblin.AttackModifier = 4;
        Goblin.InitiativeModifier = 2;
        Goblin.Weapon.Name = TEXT("Scimitar");
        Goblin.Weapon.DamageDiceCount = 1;
        Goblin.Weapon.DamageDieSize = 6;
        Goblin.Weapon.AttackAbility = EGCAbility::Dexterity;
        Goblin.Weapon.bProficient = true;
        return Goblin;
    }
};
