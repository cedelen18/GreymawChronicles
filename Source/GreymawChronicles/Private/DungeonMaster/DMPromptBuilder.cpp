#include "DungeonMaster/DMPromptBuilder.h"

FString UDMPromptBuilder::BuildDMPrompt(const FString& PlayerInput, const FDMPromptContext& Context) const
{
    FString Prompt;
    Prompt += BuildStaticSections();
    Prompt += TEXT("\nSECTION 4: CURRENT SCENE CONTEXT\n") + Context.SceneContext + TEXT("\n");
    Prompt += TEXT("\nSECTION 5: CHARACTER SHEETS\n") + Context.CharacterSheets + TEXT("\n");
    Prompt += TEXT("\nSECTION 6: ADVENTURE STATE\n") + Context.AdventureState + TEXT("\n");
    Prompt += TEXT("\nSECTION 7: CONVERSATION HISTORY\n") + Context.ConversationHistory + TEXT("\n");
    Prompt += TEXT("\nPLAYER INPUT:\n") + PlayerInput + TEXT("\n");
    return Prompt;
}

FString UDMPromptBuilder::BuildCompanionCombatPrompt(const FString& CombatStateJson) const
{
    return FString::Printf(TEXT("You are Kael, a tactical fighter companion. Return JSON only.\nCombat State:\n%s\n"), *CombatStateJson);
}

FString UDMPromptBuilder::BuildStaticSections() const
{
    return TEXT("SECTION 1: DM PERSONALITY AND RULES\n")
        TEXT("Be cinematic and fair.\n")
        TEXT("\nSECTION 2: ACTION VOCABULARY\n")
        TEXT("Use allowed action verbs only.\n")
        TEXT("\nSECTION 3: OUTPUT FORMAT SCHEMA\n")
        TEXT("Return strict JSON compatible with FDMResponse.\n");
}
