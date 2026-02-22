#include "DungeonMaster/DMBrainSubsystem.h"

#include "DungeonMaster/DMConversationHistory.h"
#include "DungeonMaster/DMPromptBuilder.h"
#include "DungeonMaster/DMResponseParser.h"
#include "Gameplay/GCCharacterSheet.h"
#include "Gameplay/DiceRoller.h"
#include "Ollama/OllamaSubsystem.h"
#include "Rules/CombatResolver.h"
#include "Rules/SpellSystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogDMBrainSubsystem, Log, All);

void UDMBrainSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    PromptBuilder = NewObject<UDMPromptBuilder>(this);
    ResponseParser = NewObject<UDMResponseParser>(this);
    ConversationHistory = NewObject<UDMConversationHistory>(this);
    OllamaSubsystem = GetGameInstance()->GetSubsystem<UOllamaSubsystem>();

    DiceRoller = NewObject<UDiceRoller>(this);
    AbilityCheckResolver = NewObject<UAbilityCheckResolver>(this);
    AbilityCheckResolver->Initialize(DiceRoller);
    CombatResolver = NewObject<UCombatResolver>(this);
    CombatResolver->Initialize(DiceRoller);
    SpellSystem = NewObject<USpellSystem>(this);
    SpellSystem->Initialize(DiceRoller);
}

void UDMBrainSubsystem::SetRuleContext(UGCCharacterSheet* InPlayerSheet)
{
    PlayerSheet = InPlayerSheet;
}

void UDMBrainSubsystem::ProcessPlayerInput(const FString& PlayerInput)
{
    if (!OllamaSubsystem || !PromptBuilder || !ResponseParser || !ConversationHistory)
    {
        UE_LOG(LogDMBrainSubsystem, Error, TEXT("DM Brain dependencies are missing."));
        return;
    }

    FDMPromptContext Context;
    Context.SceneContext = TEXT("scene_id=tavern_arrival");
    Context.CharacterSheets = TEXT("{}");
    Context.AdventureState = TEXT("{}");
    Context.ConversationHistory = ConversationHistory->ToPromptString();

    const FString Prompt = PromptBuilder->BuildDMPrompt(PlayerInput, Context);

    FOllamaRequestOptions Options;
    Options.Priority = EOllamaRequestPriority::High;
    Options.Temperature = 0.7f;

    OllamaSubsystem->SendGenerateRequest(TEXT("qwen2.5:32b"), Prompt, Options,
        FOnOllamaComplete::CreateUObject(this, &UDMBrainSubsystem::OnOllamaCompletion, PlayerInput));
}

void UDMBrainSubsystem::OnOllamaCompletion(bool bSuccess, const FString& ResponseText, float LatencySeconds, FString OriginalPlayerInput)
{
    if (!bSuccess)
    {
        UE_LOG(LogDMBrainSubsystem, Warning, TEXT("DM request failed in %.2fs: %s"), LatencySeconds, *ResponseText);
        OnDMNarration.Broadcast(TEXT("The DM falters for a moment. Please try again."));
        return;
    }

    const FDMResponse Parsed = ResponseParser->ParseResponse(ResponseText);
    if (!Parsed.bValid)
    {
        UE_LOG(LogDMBrainSubsystem, Warning, TEXT("DM parse failure: %s"), *Parsed.Error);
        OnDMNarration.Broadcast(TEXT("I couldn't interpret that response cleanly. Try rephrasing your action."));
        return;
    }

    FDMExchange Exchange;
    Exchange.PlayerInput = OriginalPlayerInput;
    Exchange.DMResponseSummary = Parsed.Narration.Left(250);
    Exchange.SceneID = TEXT("tavern_arrival");
    Exchange.TimestampUtc = FDateTime::UtcNow();
    ConversationHistory->AddExchange(Exchange);

    ResolveParsedResponse(Parsed);
}

EGCSkill UDMBrainSubsystem::ResolveCheckSkill(const FString& CheckType) const
{
    const FString Lower = CheckType.ToLower();
    if (Lower.Contains(TEXT("persuasion"))) return EGCSkill::Persuasion;
    if (Lower.Contains(TEXT("perception"))) return EGCSkill::Perception;
    if (Lower.Contains(TEXT("athletics"))) return EGCSkill::Athletics;
    if (Lower.Contains(TEXT("stealth"))) return EGCSkill::Stealth;
    if (Lower.Contains(TEXT("arcana"))) return EGCSkill::Arcana;
    if (Lower.Contains(TEXT("intimidation"))) return EGCSkill::Intimidation;
    return EGCSkill::Athletics;
}

void UDMBrainSubsystem::ResolveParsedResponse(const FDMResponse& Parsed)
{
    if (!DiceRoller)
    {
        DiceRoller = NewObject<UDiceRoller>(this);
    }
    if (!AbilityCheckResolver)
    {
        AbilityCheckResolver = NewObject<UAbilityCheckResolver>(this);
        AbilityCheckResolver->Initialize(DiceRoller);
    }
    if (!CombatResolver)
    {
        CombatResolver = NewObject<UCombatResolver>(this);
        CombatResolver->Initialize(DiceRoller);
    }

    OnDMNarration.Broadcast(Parsed.Narration);

    if (!Parsed.Check.bCheckRequired)
    {
        OnDMActionsReady.Broadcast(Parsed.Actions);
        ApplyWorldChanges(Parsed.WorldChanges);
        return;
    }

    if (!PlayerSheet || !AbilityCheckResolver)
    {
        UE_LOG(LogDMBrainSubsystem, Warning, TEXT("Check requested but no rules context available."));
        OnDMNarration.Broadcast(Parsed.FailureBranch.Narration.IsEmpty() ? Parsed.Narration : Parsed.FailureBranch.Narration);
        OnDMActionsReady.Broadcast(Parsed.FailureBranch.Actions);
        return;
    }

    const FAbilityCheckResult CheckResult = AbilityCheckResolver->ResolveAbilityCheck(PlayerSheet, ResolveCheckSkill(Parsed.Check.CheckType), Parsed.Check.DC, false, false);
    OnDMDiceResolved.Broadcast(CheckResult);

    if (CheckResult.bSuccess)
    {
        OnDMNarration.Broadcast(Parsed.SuccessBranch.Narration);
        OnDMActionsReady.Broadcast(Parsed.SuccessBranch.Actions);
    }
    else
    {
        OnDMNarration.Broadcast(Parsed.FailureBranch.Narration);
        OnDMActionsReady.Broadcast(Parsed.FailureBranch.Actions);
    }

    ApplyWorldChanges(Parsed.WorldChanges);
}

void UDMBrainSubsystem::ApplyWorldChanges(const TArray<FDMWorldChange>& WorldChanges)
{
    if (!PlayerSheet || !CombatResolver)
    {
        return;
    }

    for (const FDMWorldChange& Change : WorldChanges)
    {
        const FString Type = Change.Type.ToLower();
        if (Change.Key != TEXT("player"))
        {
            continue;
        }

        const int32 Amount = FCString::Atoi(*Change.Value);
        if (Type == TEXT("damage"))
        {
            CombatResolver->ApplyDamage(PlayerSheet, Amount);
        }
        else if (Type == TEXT("heal"))
        {
            PlayerSheet->CurrentHP = FMath::Clamp(PlayerSheet->CurrentHP + FMath::Max(0, Amount), 0, PlayerSheet->MaxHP);
        }
    }
}
