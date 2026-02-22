#include "DungeonMaster/DMBrainSubsystem.h"

#include "DungeonMaster/DMConversationHistory.h"
#include "DungeonMaster/DMIntentClassifier.h"
#include "DungeonMaster/DMNarrationPool.h"
#include "DungeonMaster/DMPromptBuilder.h"
#include "DungeonMaster/DMResponseParser.h"
#include "DungeonMaster/DMWorldStateSubsystem.h"
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

    IntentClassifier = NewObject<UDMIntentClassifier>(this);
    NarrationPool = NewObject<UDMNarrationPool>(this);
    NarrationPool->PopulateTavernDefaults();

    // Sprint I: World state subsystem for NPC dispositions
    WorldState = GetGameInstance() ? GetGameInstance()->GetSubsystem<UDMWorldStateSubsystem>() : nullptr;
}

void UDMBrainSubsystem::SetRuleContext(UGCCharacterSheet* InPlayerSheet)
{
    PlayerSheet = InPlayerSheet;
}

void UDMBrainSubsystem::SetProcessingState(bool bNewState)
{
    UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;

    if (bNewState)
    {
        // Cancel any pending debounce — we're starting a new processing cycle
        if (World)
        {
            World->GetTimerManager().ClearTimer(DebounceHandle);
        }

        if (bIsProcessing)
        {
            return; // Already processing
        }

        bIsProcessing = true;
        OnDMProcessingStateChanged.Broadcast(true);

        // Sprint I: Safety timeout — force-reset if processing takes too long (20s)
        if (World)
        {
            World->GetTimerManager().SetTimer(ProcessingTimeoutHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
            {
                UE_LOG(LogDMBrainSubsystem, Warning, TEXT("Processing state timed out after 20s. Force-resetting."));
                bIsProcessing = false;
                OnDMProcessingStateChanged.Broadcast(false);
                OnDMNarration.Broadcast(TEXT("The DM seems to have lost their train of thought. Try again."));
            }), 20.0f, false);
        }
    }
    else
    {
        if (!bIsProcessing)
        {
            return; // Already idle
        }

        // Clear the safety timeout
        if (World)
        {
            World->GetTimerManager().ClearTimer(ProcessingTimeoutHandle);
        }

        // Sprint I: Debounce — delay the actual idle transition by 0.3s to prevent double-submit
        if (World)
        {
            World->GetTimerManager().SetTimer(DebounceHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
            {
                bIsProcessing = false;
                OnDMProcessingStateChanged.Broadcast(false);
            }), 0.3f, false);
        }
        else
        {
            bIsProcessing = false;
            OnDMProcessingStateChanged.Broadcast(false);
        }
    }
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
    if (!IntentClassifier || !NarrationPool)
    {
        return false;
    }

    const FDMIntentResult Intent = IntentClassifier->Classify(PlayerInput);
    return HandleIntentScripted(Intent, PlayerInput);
}

bool UDMBrainSubsystem::HandleIntentScripted(const FDMIntentResult& Intent, const FString& RawInput)
{
    if (!NarrationPool)
    {
        return false;
    }

    FDMResponse Scripted;
    Scripted.bValid = true;

    const FString Lower = RawInput.ToLower();
    const FString& Subject = Intent.Subject;

    // ===================================================================
    // INTENT: LOOK / OBSERVE
    // ===================================================================
    if (Intent.Intent == EDMIntent::Look)
    {
        // Subject-specific look
        if (Subject.Contains(TEXT("hearth")) || Subject.Contains(TEXT("fire")))
        {
            Scripted.Narration = NarrationPool->PickRandom(TEXT("inspect_hearth"));
        }
        else
        {
            Scripted.Narration = NarrationPool->PickRandom(TEXT("look_around"));
        }

        FDMAction LookAction;
        LookAction.Action = TEXT("idle");
        LookAction.Actor = TEXT("player");
        LookAction.DelaySeconds = 1.0f;
        Scripted.Actions.Add(LookAction);

        ResolveParsedResponse(Scripted);
        return true;
    }

    // ===================================================================
    // INTENT: TALK / SPEAK / ASK
    // ===================================================================
    if (Intent.Intent == EDMIntent::Talk)
    {
        if (Subject.Contains(TEXT("marta")))
        {
            // Sprint I: Disposition-aware narration for Marta
            const FString MartaDisposition = WorldState ? WorldState->GetState(TEXT("npc_disposition"), TEXT("marta")) : FString();
            if (MartaDisposition == TEXT("friendly"))
            {
                Scripted.Narration = NarrationPool->PickRandom(TEXT("talk_marta_friendly"));
            }
            else if (MartaDisposition == TEXT("suspicious"))
            {
                Scripted.Narration = NarrationPool->PickRandom(TEXT("talk_marta_suspicious"));
            }
            else if (MartaDisposition == TEXT("trusting"))
            {
                Scripted.Narration = NarrationPool->PickRandom(TEXT("talk_marta_friendly"));
            }
            else
            {
                Scripted.Narration = NarrationPool->PickRandom(TEXT("talk_marta"));
            }

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

        if (Subject.Contains(TEXT("kael")))
        {
            Scripted.Narration = NarrationPool->PickRandom(TEXT("talk_kael"));

            FDMAction TalkAction;
            TalkAction.Action = TEXT("talk_gesture");
            TalkAction.Actor = TEXT("kael");
            TalkAction.Target = TEXT("player");
            TalkAction.DelaySeconds = 0.8f;
            Scripted.Actions.Add(TalkAction);

            ResolveParsedResponse(Scripted);
            return true;
        }

        if (Subject.Contains(TEXT("durgan")) || Subject.Contains(TEXT("old man")))
        {
            // Sprint I: Disposition-aware narration for Durgan
            const FString DurganDisposition = WorldState ? WorldState->GetState(TEXT("npc_disposition"), TEXT("durgan")) : FString();
            if (DurganDisposition == TEXT("open"))
            {
                Scripted.Narration = NarrationPool->PickRandom(TEXT("talk_durgan_open"));
            }
            else
            {
                Scripted.Narration = NarrationPool->PickRandom(TEXT("talk_durgan"));
            }

            FDMAction TalkAction;
            TalkAction.Action = TEXT("talk_gesture");
            TalkAction.Actor = TEXT("durgan");
            TalkAction.Target = TEXT("player");
            TalkAction.DelaySeconds = 1.0f;
            Scripted.Actions.Add(TalkAction);

            ResolveParsedResponse(Scripted);
            return true;
        }

        // Generic talk with no specific subject — reminder
        Scripted.Narration = NarrationPool->PickRandom(TEXT("fallback"));
        FDMAction IdleAction;
        IdleAction.Action = TEXT("idle");
        IdleAction.Actor = TEXT("player");
        IdleAction.DelaySeconds = 0.5f;
        Scripted.Actions.Add(IdleAction);
        ResolveParsedResponse(Scripted);
        return true;
    }

    // ===================================================================
    // INTENT: MOVE / GO
    // ===================================================================
    if (Intent.Intent == EDMIntent::Move)
    {
        if (Subject.Contains(TEXT("bar")))
        {
            Scripted.Narration = NarrationPool->PickRandom(TEXT("move_bar"));

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

        if (Subject.Contains(TEXT("door")))
        {
            Scripted.Narration = TEXT("You move toward the tavern door. The rain hammers against the other side. A cold draft slips through the frame. The Greymaw trail starts just beyond the village gate.");

            FDMAction MoveAction;
            MoveAction.Action = TEXT("move");
            MoveAction.Actor = TEXT("player");
            MoveAction.bHasMoveTarget = true;
            MoveAction.MoveTarget = FVector(-300.0f, 200.0f, 100.0f);
            MoveAction.MoveSpeedUnitsPerSecond = 200.0f;
            Scripted.Actions.Add(MoveAction);

            ResolveParsedResponse(Scripted);
            return true;
        }

        if (Subject.Contains(TEXT("hearth")) || Subject.Contains(TEXT("fire")))
        {
            Scripted.Narration = TEXT("You cross toward the hearth. The warmth wraps around you like a cloak. From here you can see the whole room.");

            FDMAction MoveAction;
            MoveAction.Action = TEXT("move");
            MoveAction.Actor = TEXT("player");
            MoveAction.bHasMoveTarget = true;
            MoveAction.MoveTarget = FVector(-200.0f, -150.0f, 100.0f);
            MoveAction.MoveSpeedUnitsPerSecond = 200.0f;
            Scripted.Actions.Add(MoveAction);

            ResolveParsedResponse(Scripted);
            return true;
        }

        // Generic move fallback
        Scripted.Narration = TEXT("You shift your position in the taproom, taking in the scene from a new angle.");
        FDMAction IdleAction;
        IdleAction.Action = TEXT("idle");
        IdleAction.Actor = TEXT("player");
        IdleAction.DelaySeconds = 0.5f;
        Scripted.Actions.Add(IdleAction);
        ResolveParsedResponse(Scripted);
        return true;
    }

    // ===================================================================
    // INTENT: INSPECT / EXAMINE / READ
    // ===================================================================
    if (Intent.Intent == EDMIntent::Inspect)
    {
        if (Subject.Contains(TEXT("board")) || Subject.Contains(TEXT("quest")) || Subject.Contains(TEXT("notice")))
        {
            Scripted.Narration = NarrationPool->PickRandom(TEXT("inspect_board"));

            FDMAction LookAction;
            LookAction.Action = TEXT("idle");
            LookAction.Actor = TEXT("player");
            LookAction.DelaySeconds = 1.5f;
            Scripted.Actions.Add(LookAction);

            ResolveParsedResponse(Scripted);
            return true;
        }

        if (Subject.Contains(TEXT("hearth")) || Subject.Contains(TEXT("fire")))
        {
            Scripted.Narration = NarrationPool->PickRandom(TEXT("inspect_hearth"));

            FDMAction LookAction;
            LookAction.Action = TEXT("idle");
            LookAction.Actor = TEXT("player");
            LookAction.DelaySeconds = 1.0f;
            Scripted.Actions.Add(LookAction);

            ResolveParsedResponse(Scripted);
            return true;
        }

        // MICRO-EVENT 1: Inspect the ale / mug — Perception check to notice something in the drink
        if (Subject.Contains(TEXT("ale")) || Subject.Contains(TEXT("mug")) || Subject.Contains(TEXT("drink")))
        {
            Scripted.Narration = TEXT("You peer into the dark ale. Something catches your eye — a faint shimmer beneath the foam. Is it just a trick of the light?");
            Scripted.Check.bCheckRequired = true;
            Scripted.Check.CheckType = TEXT("perception");
            Scripted.Check.DC = 12;

            Scripted.SuccessBranch.Narration = TEXT("You spot a tiny rune etched into the bottom of the mug — a ward against poison. Marta's been careful lately. She nods when she sees you noticed.");
            FDMAction SuccessAction;
            SuccessAction.Action = TEXT("nod");
            SuccessAction.Actor = TEXT("marta");
            SuccessAction.Target = TEXT("player");
            SuccessAction.DelaySeconds = 0.5f;
            Scripted.SuccessBranch.Actions.Add(SuccessAction);

            Scripted.FailureBranch.Narration = TEXT("The shimmer fades. Probably just the lamplight on the foam. You take a sip — tastes like regular ale, if a bit flat.");
            FDMAction FailAction;
            FailAction.Action = TEXT("idle");
            FailAction.Actor = TEXT("player");
            FailAction.DelaySeconds = 0.5f;
            Scripted.FailureBranch.Actions.Add(FailAction);

            ResolveParsedResponse(Scripted);
            return true;
        }

        // Generic inspect
        Scripted.Narration = TEXT("You take a closer look, but nothing out of the ordinary stands out. The taproom carries on as usual.");
        FDMAction IdleAction;
        IdleAction.Action = TEXT("idle");
        IdleAction.Actor = TEXT("player");
        IdleAction.DelaySeconds = 0.8f;
        Scripted.Actions.Add(IdleAction);
        ResolveParsedResponse(Scripted);
        return true;
    }

    // ===================================================================
    // INTENT: CHALLENGE / FIGHT / CONTEST
    // ===================================================================
    if (Intent.Intent == EDMIntent::Challenge)
    {
        // Arm wrestle / challenge Kael (original)
        if (Subject.Contains(TEXT("kael")) || Lower.Contains(TEXT("arm wrestle")) || Lower.Contains(TEXT("wrestle")))
        {
            Scripted.Narration = NarrationPool->PickRandom(TEXT("challenge_kael_setup"));
            Scripted.Check.bCheckRequired = true;
            Scripted.Check.CheckType = TEXT("athletics");
            Scripted.Check.DC = 14;

            Scripted.SuccessBranch.Narration = NarrationPool->PickRandom(TEXT("challenge_kael_win"));
            FDMAction SuccessGesture;
            SuccessGesture.Action = TEXT("talk_gesture");
            SuccessGesture.Actor = TEXT("kael");
            SuccessGesture.Target = TEXT("player");
            SuccessGesture.DelaySeconds = 0.5f;
            Scripted.SuccessBranch.Actions.Add(SuccessGesture);

            Scripted.FailureBranch.Narration = NarrationPool->PickRandom(TEXT("challenge_kael_lose"));
            FDMAction FailGesture;
            FailGesture.Action = TEXT("shake_head");
            FailGesture.Actor = TEXT("kael");
            FailGesture.Target = TEXT("player");
            FailGesture.DelaySeconds = 0.5f;
            Scripted.FailureBranch.Actions.Add(FailGesture);

            ResolveParsedResponse(Scripted);
            return true;
        }

        // MICRO-EVENT 2: Intimidate a rowdy patron — Intimidation check (pure narrative, no combat)
        Scripted.Narration = TEXT("A heavyset patron at the corner table has been getting louder with each drink. He knocks over a mug and sneers at the room. You step forward, meeting his gaze.");
        Scripted.Check.bCheckRequired = true;
        Scripted.Check.CheckType = TEXT("intimidation");
        Scripted.Check.DC = 13;

        Scripted.SuccessBranch.Narration = TEXT("The patron falters under your stare. He mumbles an apology, rights the mug, and shrinks back into his seat. Marta gives you a grateful nod from behind the bar.");
        FDMAction SuccessNod;
        SuccessNod.Action = TEXT("nod");
        SuccessNod.Actor = TEXT("marta");
        SuccessNod.Target = TEXT("player");
        SuccessNod.DelaySeconds = 0.5f;
        Scripted.SuccessBranch.Actions.Add(SuccessNod);

        Scripted.FailureBranch.Narration = TEXT("The patron snorts. 'Mind your business, outsider.' He turns back to his drink, unbothered. A few patrons glance your way with mild amusement.");
        FDMAction FailIdle;
        FailIdle.Action = TEXT("idle");
        FailIdle.Actor = TEXT("player");
        FailIdle.DelaySeconds = 0.6f;
        Scripted.FailureBranch.Actions.Add(FailIdle);

        ResolveParsedResponse(Scripted);
        return true;
    }

    // ===================================================================
    // INTENT: HELP / ASSIST
    // ===================================================================
    if (Intent.Intent == EDMIntent::Help)
    {
        if (Subject.Contains(TEXT("durgan")) || Subject.Contains(TEXT("old man")))
        {
            Scripted.Narration = NarrationPool->PickRandom(TEXT("help_durgan"));

            FDMAction TalkAction;
            TalkAction.Action = TEXT("talk_gesture");
            TalkAction.Actor = TEXT("player");
            TalkAction.Target = TEXT("durgan");
            TalkAction.DelaySeconds = 0.8f;
            Scripted.Actions.Add(TalkAction);

            ResolveParsedResponse(Scripted);
            return true;
        }

        // Generic help — pure narrative path (no check)
        Scripted.Narration = TEXT("You offer a hand to no one in particular. Marta glances over. 'If you want to help, look into those disappearances. That would be more use than anything else.'");
        FDMAction IdleAction;
        IdleAction.Action = TEXT("idle");
        IdleAction.Actor = TEXT("player");
        IdleAction.DelaySeconds = 0.5f;
        Scripted.Actions.Add(IdleAction);
        ResolveParsedResponse(Scripted);
        return true;
    }

    // ===================================================================
    // INTENT: USE / INTERACT
    // ===================================================================
    if (Intent.Intent == EDMIntent::Use)
    {
        // MICRO-EVENT 3: Pick up / grab a suspicious coin on the floor — Sleight of Hand check
        if (Subject.Contains(TEXT("drink")) || Subject.Contains(TEXT("ale")) || Subject.Contains(TEXT("mug")))
        {
            Scripted.Narration = NarrationPool->PickRandom(TEXT("use_drink"));

            FDMAction DrinkAction;
            DrinkAction.Action = TEXT("idle");
            DrinkAction.Actor = TEXT("player");
            DrinkAction.DelaySeconds = 1.0f;
            Scripted.Actions.Add(DrinkAction);

            ResolveParsedResponse(Scripted);
            return true;
        }

        // Interact with door
        if (Subject.Contains(TEXT("door")))
        {
            Scripted.Narration = TEXT("You push the tavern door open a crack. Cold rain blows in and the wind carries a sound — distant, rhythmic, like breathing. The Greymaw trail awaits, but the night is dark.");

            FDMAction IdleAction;
            IdleAction.Action = TEXT("idle");
            IdleAction.Actor = TEXT("player");
            IdleAction.DelaySeconds = 1.0f;
            Scripted.Actions.Add(IdleAction);

            ResolveParsedResponse(Scripted);
            return true;
        }

        // Pick up coin on the floor (generic "use" / "grab" / "pick up" with no obvious subject)
        Scripted.Narration = TEXT("You notice a glint under a nearby table — a coin, half-buried in grime. You reach for it, but another hand darts out from a shadowed booth.");
        Scripted.Check.bCheckRequired = true;
        Scripted.Check.CheckType = TEXT("sleight_of_hand");
        Scripted.Check.DC = 11;

        Scripted.SuccessBranch.Narration = TEXT("Your fingers close around the coin first. It's heavier than it should be — not copper, but something older. The hand retreats into the shadows without a word.");
        FDMAction SuccessAction;
        SuccessAction.Action = TEXT("idle");
        SuccessAction.Actor = TEXT("player");
        SuccessAction.DelaySeconds = 0.5f;
        Scripted.SuccessBranch.Actions.Add(SuccessAction);

        // World change: award the mysterious coin
        FDMWorldChange CoinChange;
        CoinChange.Type = TEXT("inventory_add");
        CoinChange.Key = TEXT("player");
        CoinChange.Value = TEXT("mysterious_coin");
        Scripted.SuccessBranch.Actions[0].DelaySeconds = 0.5f;

        Scripted.FailureBranch.Narration = TEXT("The shadowed hand snatches the coin away before you can grab it. You hear a dry chuckle from the booth, then silence.");
        FDMAction FailAction;
        FailAction.Action = TEXT("idle");
        FailAction.Actor = TEXT("player");
        FailAction.DelaySeconds = 0.5f;
        Scripted.FailureBranch.Actions.Add(FailAction);

        ResolveParsedResponse(Scripted);
        return true;
    }

    // ===================================================================
    // INTENT: ORDER / BUY (Sprint I)
    // ===================================================================
    if (Intent.Intent == EDMIntent::Order)
    {
        // Check if Marta is suspicious — refuses service
        const FString MartaDisp = WorldState ? WorldState->GetState(TEXT("npc_disposition"), TEXT("marta")) : FString();
        if (MartaDisp == TEXT("suspicious"))
        {
            Scripted.Narration = NarrationPool->PickRandom(TEXT("order_refused"));
            FDMAction ShakeHead;
            ShakeHead.Action = TEXT("shake_head");
            ShakeHead.Actor = TEXT("marta");
            ShakeHead.Target = TEXT("player");
            ShakeHead.DelaySeconds = 0.5f;
            Scripted.Actions.Add(ShakeHead);
            ResolveParsedResponse(Scripted);
            return true;
        }

        // Subject ale/drink — redirect to existing use_drink
        if (Subject.Contains(TEXT("ale")) || Subject.Contains(TEXT("drink")) || Subject.Contains(TEXT("mug")))
        {
            Scripted.Narration = NarrationPool->PickRandom(TEXT("use_drink"));
        }
        else
        {
            Scripted.Narration = NarrationPool->PickRandom(TEXT("order_food"));
        }

        FDMAction ServeAction;
        ServeAction.Action = TEXT("nod");
        ServeAction.Actor = TEXT("marta");
        ServeAction.Target = TEXT("player");
        ServeAction.DelaySeconds = 0.6f;
        Scripted.Actions.Add(ServeAction);

        // Stateful consequence: Marta becomes friendly
        FDMWorldChange Disposition;
        Disposition.Type = TEXT("npc_disposition");
        Disposition.Key = TEXT("marta");
        Disposition.Value = TEXT("friendly");
        Scripted.WorldChanges.Add(Disposition);

        ResolveParsedResponse(Scripted);
        return true;
    }

    // ===================================================================
    // INTENT: STEAL / PICKPOCKET (Sprint I)
    // ===================================================================
    if (Intent.Intent == EDMIntent::Steal)
    {
        Scripted.Narration = NarrationPool->PickRandom(TEXT("steal_setup"));
        Scripted.Check.bCheckRequired = true;
        Scripted.Check.CheckType = TEXT("sleight_of_hand");

        // DC varies by target
        FString StealTarget = TEXT("patron");
        if (Subject.Contains(TEXT("marta")))
        {
            Scripted.Check.DC = 14;
            StealTarget = TEXT("marta");
        }
        else if (Subject.Contains(TEXT("kael")))
        {
            Scripted.Check.DC = 16;
            StealTarget = TEXT("kael");
        }
        else if (Subject.Contains(TEXT("durgan")))
        {
            Scripted.Check.DC = 10;
            StealTarget = TEXT("durgan");
        }
        else
        {
            Scripted.Check.DC = 13;
        }

        Scripted.SuccessBranch.Narration = NarrationPool->PickRandom(TEXT("steal_success"));
        FDMAction SuccessAction;
        SuccessAction.Action = TEXT("idle");
        SuccessAction.Actor = TEXT("player");
        SuccessAction.DelaySeconds = 0.5f;
        Scripted.SuccessBranch.Actions.Add(SuccessAction);

        FDMWorldChange LootChange;
        LootChange.Type = TEXT("inventory_add");
        LootChange.Key = TEXT("player");
        LootChange.Value = TEXT("stolen_coin_pouch");
        Scripted.WorldChanges.Add(LootChange);

        Scripted.FailureBranch.Narration = NarrationPool->PickRandom(TEXT("steal_fail"));
        FDMAction FailAction;
        FailAction.Action = TEXT("shake_head");
        FailAction.Actor = *StealTarget;
        FailAction.Target = TEXT("player");
        FailAction.DelaySeconds = 0.6f;
        Scripted.FailureBranch.Actions.Add(FailAction);

        // Stateful consequence on failure: target becomes suspicious
        FDMWorldChange SuspiciousChange;
        SuspiciousChange.Type = TEXT("npc_disposition");
        SuspiciousChange.Key = StealTarget;
        SuspiciousChange.Value = TEXT("suspicious");
        Scripted.WorldChanges.Add(SuspiciousChange);

        ResolveParsedResponse(Scripted);
        return true;
    }

    // ===================================================================
    // INTENT: LISTEN / EAVESDROP (Sprint I)
    // ===================================================================
    if (Intent.Intent == EDMIntent::Listen)
    {
        Scripted.Narration = TEXT("You lean back and focus on the sounds of the taproom, filtering out the rain and the crackle of the hearth.");
        Scripted.Check.bCheckRequired = true;
        Scripted.Check.CheckType = TEXT("perception");
        Scripted.Check.DC = 12;

        Scripted.SuccessBranch.Narration = NarrationPool->PickRandom(TEXT("listen_success"));
        FDMAction SuccessAction;
        SuccessAction.Action = TEXT("idle");
        SuccessAction.Actor = TEXT("player");
        SuccessAction.DelaySeconds = 0.8f;
        Scripted.SuccessBranch.Actions.Add(SuccessAction);

        Scripted.FailureBranch.Narration = NarrationPool->PickRandom(TEXT("listen_fail"));
        FDMAction FailAction;
        FailAction.Action = TEXT("idle");
        FailAction.Actor = TEXT("player");
        FailAction.DelaySeconds = 0.5f;
        Scripted.FailureBranch.Actions.Add(FailAction);

        ResolveParsedResponse(Scripted);
        return true;
    }

    // ===================================================================
    // INTENT: PERSUADE / CONVINCE (Sprint I)
    // ===================================================================
    if (Intent.Intent == EDMIntent::Persuade)
    {
        Scripted.Check.bCheckRequired = true;
        Scripted.Check.CheckType = TEXT("persuasion");

        FString PersuadeTarget = TEXT("player");
        if (Subject.Contains(TEXT("marta")))
        {
            Scripted.Narration = TEXT("You lean on the bar and try to charm Marta with your best persuasive tone.");
            Scripted.Check.DC = 12;
            PersuadeTarget = TEXT("marta");

            Scripted.SuccessBranch.Narration = NarrationPool->PickRandom(TEXT("persuade_marta_success"));

            FDMWorldChange TrustChange;
            TrustChange.Type = TEXT("npc_disposition");
            TrustChange.Key = TEXT("marta");
            TrustChange.Value = TEXT("trusting");
            Scripted.WorldChanges.Add(TrustChange);
        }
        else if (Subject.Contains(TEXT("durgan")) || Subject.Contains(TEXT("old man")))
        {
            Scripted.Narration = TEXT("You pull up a chair beside Durgan and try to coax more of his story out of him.");
            Scripted.Check.DC = 14;
            PersuadeTarget = TEXT("durgan");

            Scripted.SuccessBranch.Narration = NarrationPool->PickRandom(TEXT("persuade_durgan_success"));

            FDMWorldChange OpenChange;
            OpenChange.Type = TEXT("npc_disposition");
            OpenChange.Key = TEXT("durgan");
            OpenChange.Value = TEXT("open");
            Scripted.WorldChanges.Add(OpenChange);
        }
        else if (Subject.Contains(TEXT("kael")))
        {
            Scripted.Narration = TEXT("You sidle up to Kael and try to talk him into sharing what he knows.");
            Scripted.Check.DC = 10;
            PersuadeTarget = TEXT("kael");

            Scripted.SuccessBranch.Narration = TEXT("Kael relents with a half-smile. 'Fine. I've heard things on the trail — sounds that shouldn't be there. Like the earth itself is groaning. Whatever took those villagers, it's not natural.'");
        }
        else
        {
            Scripted.Narration = TEXT("You try to persuade the room in general, but no one in particular seems to be your target.");
            Scripted.Check.DC = 13;
            Scripted.SuccessBranch.Narration = TEXT("A passing patron stops and shares a rumor about markings on the Greymaw trail — old symbols that glow at dusk.");
        }

        FDMAction SuccessAction;
        SuccessAction.Action = TEXT("talk_gesture");
        SuccessAction.Actor = PersuadeTarget;
        SuccessAction.Target = TEXT("player");
        SuccessAction.DelaySeconds = 0.6f;
        Scripted.SuccessBranch.Actions.Add(SuccessAction);

        Scripted.FailureBranch.Narration = TEXT("Your words fall flat. The target shrugs and turns away, unswayed by your argument.");
        FDMAction FailAction;
        FailAction.Action = TEXT("shake_head");
        FailAction.Actor = PersuadeTarget;
        FailAction.Target = TEXT("player");
        FailAction.DelaySeconds = 0.5f;
        Scripted.FailureBranch.Actions.Add(FailAction);

        ResolveParsedResponse(Scripted);
        return true;
    }

    // ===================================================================
    // INTENT: REST / RELAX (Sprint I)
    // ===================================================================
    if (Intent.Intent == EDMIntent::Rest)
    {
        Scripted.Narration = NarrationPool->PickRandom(TEXT("rest_tavern"));

        FDMAction RestAction;
        RestAction.Action = TEXT("idle");
        RestAction.Actor = TEXT("player");
        RestAction.DelaySeconds = 1.5f;
        Scripted.Actions.Add(RestAction);

        // If player is injured, restore 1 HP
        if (PlayerSheet && PlayerSheet->CurrentHP < PlayerSheet->MaxHP)
        {
            FDMWorldChange HealChange;
            HealChange.Type = TEXT("heal");
            HealChange.Key = TEXT("player");
            HealChange.Value = TEXT("1");
            Scripted.WorldChanges.Add(HealChange);
        }

        ResolveParsedResponse(Scripted);
        return true;
    }

    // ===================================================================
    // INTENT: GAMBLE / BET (Sprint I)
    // ===================================================================
    if (Intent.Intent == EDMIntent::Gamble)
    {
        if (Subject.Contains(TEXT("durgan")) || Subject.Contains(TEXT("old man")))
        {
            Scripted.Narration = TEXT("Durgan waves you off. 'I don't gamble anymore, traveler. Lost enough in my years.' He clutches his ale protectively.");
            FDMAction WaveOff;
            WaveOff.Action = TEXT("shake_head");
            WaveOff.Actor = TEXT("durgan");
            WaveOff.DelaySeconds = 0.5f;
            Scripted.Actions.Add(WaveOff);
            ResolveParsedResponse(Scripted);
            return true;
        }

        Scripted.Narration = NarrationPool->PickRandom(TEXT("gamble_setup"));
        Scripted.Check.bCheckRequired = true;
        Scripted.Check.CheckType = TEXT("deception");
        Scripted.Check.DC = 13;

        Scripted.SuccessBranch.Narration = NarrationPool->PickRandom(TEXT("gamble_win"));
        FDMAction WinAction;
        WinAction.Action = TEXT("talk_gesture");
        WinAction.Actor = TEXT("player");
        WinAction.DelaySeconds = 0.6f;
        Scripted.SuccessBranch.Actions.Add(WinAction);

        Scripted.FailureBranch.Narration = NarrationPool->PickRandom(TEXT("gamble_lose"));
        FDMAction LoseAction;
        LoseAction.Action = TEXT("idle");
        LoseAction.Actor = TEXT("player");
        LoseAction.DelaySeconds = 0.5f;
        Scripted.FailureBranch.Actions.Add(LoseAction);

        ResolveParsedResponse(Scripted);
        return true;
    }

    // ===================================================================
    // INTENT: UNKNOWN / FALLBACK
    // ===================================================================
    Scripted.Narration = NarrationPool->PickRandom(TEXT("fallback"));

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

    if (!ResponseParser)
    {
        UE_LOG(LogDMBrainSubsystem, Error, TEXT("ResponseParser is null in OnOllamaCompletion."));
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

    if (ConversationHistory)
    {
        FDMExchange Exchange;
        Exchange.PlayerInput = OriginalPlayerInput;
        Exchange.DMResponseSummary = Parsed.Narration.Left(250);
        Exchange.SceneID = TEXT("tavern_arrival");
        Exchange.TimestampUtc = FDateTime::UtcNow();
        ConversationHistory->AddExchange(Exchange);
    }

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
    if (Lower.Contains(TEXT("sleight_of_hand")) || Lower.Contains(TEXT("sleight of hand"))) return EGCSkill::SleightOfHand;
    if (Lower.Contains(TEXT("investigation"))) return EGCSkill::Investigation;
    if (Lower.Contains(TEXT("insight"))) return EGCSkill::Insight;
    if (Lower.Contains(TEXT("deception"))) return EGCSkill::Deception;
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
    for (const FDMWorldChange& Change : WorldChanges)
    {
        const FString Type = Change.Type.ToLower();

        // Sprint I: NPC disposition changes — stored in WorldState subsystem
        if (Type == TEXT("npc_disposition"))
        {
            if (WorldState)
            {
                WorldState->SetState(TEXT("npc_disposition"), Change.Key, Change.Value);
                UE_LOG(LogDMBrainSubsystem, Log, TEXT("NPC disposition: %s -> %s"), *Change.Key, *Change.Value);
            }
            continue;
        }

        // Player-targeted changes require PlayerSheet + CombatResolver
        if (Change.Key != TEXT("player") || !PlayerSheet || !CombatResolver)
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
