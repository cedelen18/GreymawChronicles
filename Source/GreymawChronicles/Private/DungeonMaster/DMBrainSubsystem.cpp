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
    OllamaSubsystem = GetGameInstance() ? GetGameInstance()->GetSubsystem<UOllamaSubsystem>() : nullptr;

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

void UDMBrainSubsystem::SetProcessingState(bool bNewState)
{
    if (bIsProcessing == bNewState)
    {
        return;
    }

    bIsProcessing = bNewState;
    OnDMProcessingStateChanged.Broadcast(bIsProcessing);
}

void UDMBrainSubsystem::ProcessPlayerInput(const FString& PlayerInput)
{
    if (bIsProcessing)
    {
        return;
    }

    const FString SanitizedInput = PlayerInput.TrimStartAndEnd();
    if (SanitizedInput.IsEmpty())
    {
        return;
    }

    SetProcessingState(true);

    if (bUseTavernScriptedBootstrap && TryHandleScriptedTavernPrompt(SanitizedInput))
    {
        SetProcessingState(false);
        return;
    }

    if (!OllamaSubsystem || !PromptBuilder || !ResponseParser || !ConversationHistory)
    {
        UE_LOG(LogDMBrainSubsystem, Error, TEXT("DM Brain dependencies are missing."));
        OnDMNarration.Broadcast(TEXT("The DM is not ready yet. Please try again in a moment."));
        SetProcessingState(false);
        return;
    }

    FDMPromptContext Context;
    Context.SceneContext = TEXT("scene_id=tavern_arrival; location=thornhaven_taproom; npcs=[marta,kael,durgan]");
    Context.CharacterSheets = TEXT("{}");
    Context.AdventureState = TEXT("{\"quest_stage\":\"tavern_arrival\",\"time\":\"evening\"}");
    Context.ConversationHistory = ConversationHistory->ToPromptString();

    const FString Prompt = PromptBuilder->BuildDMPrompt(SanitizedInput, Context);

    FOllamaRequestOptions Options;
    Options.Priority = EOllamaRequestPriority::High;
    Options.Temperature = 0.7f;

    OllamaSubsystem->SendGenerateRequest(TEXT("qwen2.5:32b"), Prompt, Options,
        FOnOllamaComplete::CreateUObject(this, &UDMBrainSubsystem::OnOllamaCompletion, SanitizedInput));
}

bool UDMBrainSubsystem::TryHandleScriptedTavernPrompt(const FString& PlayerInput)
{
    FDMResponse Scripted;
    Scripted.bValid = true;

    const FString Lower = PlayerInput.ToLower();

    // --- LOOK AROUND ---
    if (Lower.Contains(TEXT("look around")) || Lower.Contains(TEXT("where am i")) || Lower.Contains(TEXT("examine room")))
    {
        Scripted.Narration = TEXT("Warm lamplight spills across the Thornhaven taproom. Marta polishes a mug behind the bar, Kael leans against a post watching the room, and old Durgan mutters into his ale as rain taps the windows. A hearth crackles in the corner and a faded quest board hangs by the door.");

        FDMAction LookAction;
        LookAction.Action = TEXT("idle");
        LookAction.Actor = TEXT("player");
        LookAction.DelaySeconds = 1.0f;
        Scripted.Actions.Add(LookAction);

        ResolveParsedResponse(Scripted);
        return true;
    }

    // --- TALK TO MARTA ---
    if (Lower.Contains(TEXT("talk to marta")) || Lower.Contains(TEXT("speak to marta")) || Lower.Contains(TEXT("ask marta")))
    {
        Scripted.Narration = TEXT("Marta lowers her voice. 'Three villagers vanished near the Greymaw trail last week. The guard won't go, says it's bad luck. If you're heading that way, I'll pay fifty gold for news of what happened to them.'");

        FDMAction MoveToBar;
        MoveToBar.Action = TEXT("move");
        MoveToBar.Actor = TEXT("player");
        MoveToBar.bHasMoveTarget = true;
        MoveToBar.MoveTarget = FVector(320.0f, 0.0f, 100.0f);
        MoveToBar.MoveSpeedUnitsPerSecond = 200.0f;
        Scripted.Actions.Add(MoveToBar);

        FDMAction TalkAction;
        TalkAction.Action = TEXT("talk_gesture");
        TalkAction.Actor = TEXT("marta");
        TalkAction.Target = TEXT("player");
        TalkAction.DelaySeconds = 0.6f;
        Scripted.Actions.Add(TalkAction);

        FDMAction NodAction;
        NodAction.Action = TEXT("nod");
        NodAction.Actor = TEXT("marta");
        NodAction.DelaySeconds = 0.4f;
        Scripted.Actions.Add(NodAction);

        ResolveParsedResponse(Scripted);
        return true;
    }

    // --- WALK TO BAR ---
    if (Lower.Contains(TEXT("walk to bar")) || Lower.Contains(TEXT("move to bar")) || Lower.Contains(TEXT("go to bar")) || Lower.Contains(TEXT("approach bar")))
    {
        Scripted.Narration = TEXT("You cross the taproom floor toward the bar. Floorboards creak underfoot and conversations pause as the regulars size you up.");

        FDMAction MoveAction;
        MoveAction.Action = TEXT("move");
        MoveAction.Actor = TEXT("player");
        MoveAction.bHasMoveTarget = true;
        MoveAction.MoveTarget = FVector(350.0f, 0.0f, 100.0f);
        MoveAction.MoveSpeedUnitsPerSecond = 250.0f;
        Scripted.Actions.Add(MoveAction);

        FDMAction WaitAction;
        WaitAction.Action = TEXT("wait");
        WaitAction.Actor = TEXT("player");
        WaitAction.DelaySeconds = 0.75f;
        Scripted.Actions.Add(WaitAction);

        FDMAction IdleAction;
        IdleAction.Action = TEXT("idle");
        IdleAction.Actor = TEXT("player");
        IdleAction.DelaySeconds = 0.5f;
        Scripted.Actions.Add(IdleAction);

        ResolveParsedResponse(Scripted);
        return true;
    }

    // --- ARM WRESTLE KAEL (ability check path) ---
    if (Lower.Contains(TEXT("arm wrestle")) || Lower.Contains(TEXT("wrestle kael")) || Lower.Contains(TEXT("challenge kael")))
    {
        Scripted.Narration = TEXT("Kael grins and slams his elbow on the table. 'Let's see what you've got, adventurer.' The taproom cheers as you grip hands.");
        Scripted.Check.bCheckRequired = true;
        Scripted.Check.CheckType = TEXT("athletics");
        Scripted.Check.DC = 14;

        Scripted.SuccessBranch.Narration = TEXT("With a surge of strength, you slam Kael's hand to the table! The room erupts. Kael laughs and claps you on the shoulder. 'Stronger than you look!'");
        FDMAction SuccessGesture;
        SuccessGesture.Action = TEXT("talk_gesture");
        SuccessGesture.Actor = TEXT("kael");
        SuccessGesture.Target = TEXT("player");
        SuccessGesture.DelaySeconds = 0.5f;
        Scripted.SuccessBranch.Actions.Add(SuccessGesture);

        Scripted.FailureBranch.Narration = TEXT("Kael overpowers you with a grin and pins your arm. 'Good effort! Next round's on me.' He slides a mug your way.");
        FDMAction FailGesture;
        FailGesture.Action = TEXT("shake_head");
        FailGesture.Actor = TEXT("kael");
        FailGesture.Target = TEXT("player");
        FailGesture.DelaySeconds = 0.5f;
        Scripted.FailureBranch.Actions.Add(FailGesture);

        ResolveParsedResponse(Scripted);
        return true;
    }

    // --- TALK TO KAEL ---
    if (Lower.Contains(TEXT("talk to kael")) || Lower.Contains(TEXT("speak to kael")) || Lower.Contains(TEXT("kael")))
    {
        Scripted.Narration = TEXT("Kael straightens up. 'I've been keeping an eye on the trail north. Something doesn't feel right — too quiet. If you're going, I'm coming with you.'");

        FDMAction TalkAction;
        TalkAction.Action = TEXT("talk_gesture");
        TalkAction.Actor = TEXT("kael");
        TalkAction.Target = TEXT("player");
        TalkAction.DelaySeconds = 0.8f;
        Scripted.Actions.Add(TalkAction);

        ResolveParsedResponse(Scripted);
        return true;
    }

    // --- TALK TO DURGAN ---
    if (Lower.Contains(TEXT("durgan")) || Lower.Contains(TEXT("old man")))
    {
        Scripted.Narration = TEXT("Old Durgan peers at you with rheumy eyes. 'The Greymaw... I saw it once. Decades ago. A mouth in the earth that swallowed the screams.' He turns back to his ale, hands shaking.");

        FDMAction TalkAction;
        TalkAction.Action = TEXT("talk_gesture");
        TalkAction.Actor = TEXT("durgan");
        TalkAction.Target = TEXT("player");
        TalkAction.DelaySeconds = 1.0f;
        Scripted.Actions.Add(TalkAction);

        ResolveParsedResponse(Scripted);
        return true;
    }

    // --- EXAMINE QUEST BOARD ---
    if (Lower.Contains(TEXT("quest board")) || Lower.Contains(TEXT("notice board")) || Lower.Contains(TEXT("board")))
    {
        Scripted.Narration = TEXT("The quest board holds a single weathered parchment: 'MISSING — Three villagers last seen on the Greymaw trail. 50 gold reward for information. — Marta, Thornhaven Taproom.'");

        FDMAction LookAction;
        LookAction.Action = TEXT("idle");
        LookAction.Actor = TEXT("player");
        LookAction.DelaySeconds = 1.5f;
        Scripted.Actions.Add(LookAction);

        ResolveParsedResponse(Scripted);
        return true;
    }

    // --- CATCH-ALL: unrecognized input in scripted mode ---
    Scripted.Narration = TEXT("You pause, considering your next move. The taproom hums around you — Marta at the bar, Kael by the post, Durgan in his corner, and a quest board by the door.");

    FDMAction IdleAction;
    IdleAction.Action = TEXT("idle");
    IdleAction.Actor = TEXT("player");
    IdleAction.DelaySeconds = 0.5f;
    Scripted.Actions.Add(IdleAction);

    ResolveParsedResponse(Scripted);
    return true;
}

void UDMBrainSubsystem::OnOllamaCompletion(bool bSuccess, const FString& ResponseText, float LatencySeconds, FString OriginalPlayerInput)
{
    if (!bSuccess)
    {
        UE_LOG(LogDMBrainSubsystem, Warning, TEXT("DM request failed in %.2fs: %s"), LatencySeconds, *ResponseText);
        OnDMNarration.Broadcast(TEXT("The DM falters for a moment. Please try again."));
        SetProcessingState(false);
        return;
    }

    const FDMResponse Parsed = ResponseParser->ParseResponse(ResponseText);
    if (!Parsed.bValid)
    {
        UE_LOG(LogDMBrainSubsystem, Warning, TEXT("DM parse failure: %s"), *Parsed.Error);
        OnDMNarration.Broadcast(TEXT("I couldn't interpret that response cleanly. Try rephrasing your action."));
        SetProcessingState(false);
        return;
    }

    FDMExchange Exchange;
    Exchange.PlayerInput = OriginalPlayerInput;
    Exchange.DMResponseSummary = Parsed.Narration.Left(250);
    Exchange.SceneID = TEXT("tavern_arrival");
    Exchange.TimestampUtc = FDateTime::UtcNow();
    ConversationHistory->AddExchange(Exchange);

    ResolveParsedResponse(Parsed);
    SetProcessingState(false);
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
