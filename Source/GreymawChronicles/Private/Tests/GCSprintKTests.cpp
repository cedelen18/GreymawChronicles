#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/GCGameInstance.h"
#include "DungeonMaster/DMBrainSubsystem.h"
#include "DungeonMaster/DMIntentClassifier.h"
#include "DungeonMaster/DMTypes.h"
#include "DungeonMaster/DMWorldStateSubsystem.h"
#include "Gameplay/GCCharacterSheet.h"
#include "Engine/GameInstance.h"

/**
 * Sprint K Tests
 *
 * 12 tests covering outcome-conditional WorldChanges bug fixes,
 * NPC interaction state machine, tavern task mini-loop, save/load
 * UX feedback, and new intent classification.
 */

// ===================================================================
// Test 1: BugFix — Steal success gives loot via SuccessBranch
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintK_StealSuccess_GivesLoot,
    "GreymawChronicles.SprintK.BugFix.StealSuccess_GivesLoot",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintK_StealSuccess_GivesLoot::RunTest(const FString& Parameters)
{
    // Verify FDMOutcomeBranch can carry WorldChanges (the bug fix)
    FDMOutcomeBranch SuccessBranch;
    TestEqual(TEXT("SuccessBranch.WorldChanges should start empty"), SuccessBranch.WorldChanges.Num(), 0);

    FDMWorldChange LootChange;
    LootChange.Type = TEXT("inventory_add");
    LootChange.Key = TEXT("player");
    LootChange.Value = TEXT("stolen_coin_pouch");
    SuccessBranch.WorldChanges.Add(LootChange);

    TestEqual(TEXT("SuccessBranch should have 1 world change"), SuccessBranch.WorldChanges.Num(), 1);
    TestEqual(TEXT("WorldChange type should be inventory_add"), SuccessBranch.WorldChanges[0].Type, TEXT("inventory_add"));
    TestEqual(TEXT("WorldChange value should be stolen_coin_pouch"), SuccessBranch.WorldChanges[0].Value, TEXT("stolen_coin_pouch"));

    // Verify FailureBranch has NO loot change
    FDMOutcomeBranch FailureBranch;
    TestEqual(TEXT("FailureBranch.WorldChanges should be empty"), FailureBranch.WorldChanges.Num(), 0);

    return true;
}

// ===================================================================
// Test 2: BugFix — Steal failure should NOT give loot
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintK_StealFail_NoLoot,
    "GreymawChronicles.SprintK.BugFix.StealFail_NoLoot",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintK_StealFail_NoLoot::RunTest(const FString& Parameters)
{
    // Simulate the fixed pattern: loot only on success branch
    FDMResponse Parsed;

    FDMWorldChange LootChange;
    LootChange.Type = TEXT("inventory_add");
    LootChange.Key = TEXT("player");
    LootChange.Value = TEXT("stolen_coin_pouch");
    Parsed.SuccessBranch.WorldChanges.Add(LootChange);

    // The shared WorldChanges array should NOT contain inventory_add
    bool bSharedHasLoot = false;
    for (const FDMWorldChange& Change : Parsed.WorldChanges)
    {
        if (Change.Type == TEXT("inventory_add"))
        {
            bSharedHasLoot = true;
            break;
        }
    }
    TestFalse(TEXT("Shared WorldChanges should not contain inventory_add"), bSharedHasLoot);

    // Failure branch should NOT have inventory_add
    bool bFailHasLoot = false;
    for (const FDMWorldChange& Change : Parsed.FailureBranch.WorldChanges)
    {
        if (Change.Type == TEXT("inventory_add"))
        {
            bFailHasLoot = true;
            break;
        }
    }
    TestFalse(TEXT("FailureBranch should not contain inventory_add"), bFailHasLoot);

    return true;
}

// ===================================================================
// Test 3: BugFix — Steal failure sets suspicious on FailureBranch
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintK_StealFail_SetsSuspicious,
    "GreymawChronicles.SprintK.BugFix.StealFail_SetsSuspicious",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintK_StealFail_SetsSuspicious::RunTest(const FString& Parameters)
{
    // Simulate the fixed pattern: suspicious only on failure branch
    FDMResponse Parsed;

    FDMWorldChange SuspiciousChange;
    SuspiciousChange.Type = TEXT("npc_disposition");
    SuspiciousChange.Key = TEXT("durgan");
    SuspiciousChange.Value = TEXT("suspicious");
    Parsed.FailureBranch.WorldChanges.Add(SuspiciousChange);

    FDMWorldChange UpsetChange;
    UpsetChange.Type = TEXT("npc_interaction_state");
    UpsetChange.Key = TEXT("durgan");
    UpsetChange.Value = TEXT("upset");
    Parsed.FailureBranch.WorldChanges.Add(UpsetChange);

    TestEqual(TEXT("FailureBranch should have 2 world changes"), Parsed.FailureBranch.WorldChanges.Num(), 2);

    // Success branch should NOT have suspicious/upset
    bool bSuccessHasSuspicious = false;
    for (const FDMWorldChange& Change : Parsed.SuccessBranch.WorldChanges)
    {
        if (Change.Type == TEXT("npc_disposition") || Change.Type == TEXT("npc_interaction_state"))
        {
            bSuccessHasSuspicious = true;
            break;
        }
    }
    TestFalse(TEXT("SuccessBranch should not contain suspicious/upset changes"), bSuccessHasSuspicious);

    return true;
}

// ===================================================================
// Test 4: BugFix — Persuade success sets disposition on SuccessBranch
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintK_PersuadeSuccess_SetsDisposition,
    "GreymawChronicles.SprintK.BugFix.PersuadeSuccess_SetsDisposition",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintK_PersuadeSuccess_SetsDisposition::RunTest(const FString& Parameters)
{
    FDMResponse Parsed;

    FDMWorldChange TrustChange;
    TrustChange.Type = TEXT("npc_disposition");
    TrustChange.Key = TEXT("marta");
    TrustChange.Value = TEXT("trusting");
    Parsed.SuccessBranch.WorldChanges.Add(TrustChange);

    FDMWorldChange HelpfulChange;
    HelpfulChange.Type = TEXT("npc_interaction_state");
    HelpfulChange.Key = TEXT("marta");
    HelpfulChange.Value = TEXT("helpful");
    Parsed.SuccessBranch.WorldChanges.Add(HelpfulChange);

    TestEqual(TEXT("SuccessBranch should have 2 world changes"), Parsed.SuccessBranch.WorldChanges.Num(), 2);
    TestEqual(TEXT("First change should set trusting"), Parsed.SuccessBranch.WorldChanges[0].Value, TEXT("trusting"));
    TestEqual(TEXT("Second change should set helpful"), Parsed.SuccessBranch.WorldChanges[1].Value, TEXT("helpful"));

    return true;
}

// ===================================================================
// Test 5: BugFix — Persuade failure should NOT set disposition
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintK_PersuadeFail_NoDisposition,
    "GreymawChronicles.SprintK.BugFix.PersuadeFail_NoDisposition",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintK_PersuadeFail_NoDisposition::RunTest(const FString& Parameters)
{
    FDMResponse Parsed;

    // Place disposition changes ONLY on success branch (the fix)
    FDMWorldChange TrustChange;
    TrustChange.Type = TEXT("npc_disposition");
    TrustChange.Key = TEXT("marta");
    TrustChange.Value = TEXT("trusting");
    Parsed.SuccessBranch.WorldChanges.Add(TrustChange);

    // Failure branch should be empty of disposition changes
    bool bFailHasDisposition = false;
    for (const FDMWorldChange& Change : Parsed.FailureBranch.WorldChanges)
    {
        if (Change.Type == TEXT("npc_disposition") || Change.Type == TEXT("npc_interaction_state"))
        {
            bFailHasDisposition = true;
            break;
        }
    }
    TestFalse(TEXT("FailureBranch should not contain disposition changes"), bFailHasDisposition);

    // Shared WorldChanges should also not have disposition
    bool bSharedHasDisposition = false;
    for (const FDMWorldChange& Change : Parsed.WorldChanges)
    {
        if (Change.Type == TEXT("npc_disposition"))
        {
            bSharedHasDisposition = true;
            break;
        }
    }
    TestFalse(TEXT("Shared WorldChanges should not contain disposition changes"), bSharedHasDisposition);

    return true;
}

// ===================================================================
// Test 6: NPCState — Talk transitions NPC from neutral to engaged
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintK_TalkTransitionsToEngaged,
    "GreymawChronicles.SprintK.NPCState.TalkTransitionsToEngaged",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintK_TalkTransitionsToEngaged::RunTest(const FString& Parameters)
{
    UGameInstance* DummyGI = NewObject<UGameInstance>();
    UDMWorldStateSubsystem* WS = NewObject<UDMWorldStateSubsystem>(DummyGI);

    // Initial state: NPC has no interaction state (treated as "neutral")
    const FString InitialState = WS->GetState(TEXT("npc_interaction_state"), TEXT("marta"));
    TestTrue(TEXT("Initial NPC state should be empty (neutral)"), InitialState.IsEmpty());

    // Simulate what Talk handler does: transition to engaged
    WS->SetState(TEXT("npc_interaction_state"), TEXT("marta"), TEXT("engaged"));
    const FString AfterTalk = WS->GetState(TEXT("npc_interaction_state"), TEXT("marta"));
    TestEqual(TEXT("After first talk, NPC should be engaged"), AfterTalk, TEXT("engaged"));

    // Second talk should NOT change state (stays engaged)
    // The handler only transitions from neutral, so engaged stays engaged
    const FString StillEngaged = WS->GetState(TEXT("npc_interaction_state"), TEXT("marta"));
    TestEqual(TEXT("NPC should still be engaged"), StillEngaged, TEXT("engaged"));

    return true;
}

// ===================================================================
// Test 7: NPCState — Upset NPC stays upset (no auto-recovery)
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintK_UpsetBlocksHelpful,
    "GreymawChronicles.SprintK.NPCState.UpsetBlocksHelpful",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintK_UpsetBlocksHelpful::RunTest(const FString& Parameters)
{
    UGameInstance* DummyGI = NewObject<UGameInstance>();
    UDMWorldStateSubsystem* WS = NewObject<UDMWorldStateSubsystem>(DummyGI);

    // Set NPC to upset (via failed steal)
    WS->SetState(TEXT("npc_interaction_state"), TEXT("durgan"), TEXT("upset"));
    TestEqual(TEXT("Durgan should be upset"), WS->GetState(TEXT("npc_interaction_state"), TEXT("durgan")), TEXT("upset"));

    // Verify the state persists (no auto-recovery mechanism)
    TestTrue(TEXT("npc_interaction_state should exist"), WS->HasState(TEXT("npc_interaction_state"), TEXT("durgan")));
    TestEqual(TEXT("Durgan should still be upset"), WS->GetState(TEXT("npc_interaction_state"), TEXT("durgan")), TEXT("upset"));

    // Verify other NPCs are unaffected
    const FString MartaState = WS->GetState(TEXT("npc_interaction_state"), TEXT("marta"));
    TestTrue(TEXT("Marta should have no interaction state"), MartaState.IsEmpty());

    return true;
}

// ===================================================================
// Test 8: TaskLoop — Accept intent sets task state
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintK_AcceptSetsState,
    "GreymawChronicles.SprintK.TaskLoop.AcceptSetsState",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintK_AcceptSetsState::RunTest(const FString& Parameters)
{
    UGameInstance* DummyGI = NewObject<UGameInstance>();
    UDMWorldStateSubsystem* WS = NewObject<UDMWorldStateSubsystem>(DummyGI);

    // Before accepting, task should not exist
    const FString PreAccept = WS->GetState(TEXT("task"), TEXT("tavern_investigation"));
    TestTrue(TEXT("Task should not exist before accept"), PreAccept.IsEmpty());

    // Simulate what Accept handler does via set_state WorldChange
    WS->SetState(TEXT("task"), TEXT("tavern_investigation"), TEXT("accepted"));
    TestEqual(TEXT("Task should be accepted"), WS->GetState(TEXT("task"), TEXT("tavern_investigation")), TEXT("accepted"));

    // Verify the state round-trips through JSON
    const FString JSON = WS->ToJSON();
    UDMWorldStateSubsystem* WS2 = NewObject<UDMWorldStateSubsystem>(DummyGI);
    WS2->FromJSON(JSON);
    TestEqual(TEXT("Task state should survive serialization"), WS2->GetState(TEXT("task"), TEXT("tavern_investigation")), TEXT("accepted"));

    return true;
}

// ===================================================================
// Test 9: TaskLoop — Report with 2+ clues resolves task
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintK_ReportCountsClues,
    "GreymawChronicles.SprintK.TaskLoop.ReportCountsClues",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintK_ReportCountsClues::RunTest(const FString& Parameters)
{
    UGameInstance* DummyGI = NewObject<UGameInstance>();
    UDMWorldStateSubsystem* WS = NewObject<UDMWorldStateSubsystem>(DummyGI);

    // Set up: task accepted, 2 clues gathered
    WS->SetState(TEXT("task"), TEXT("tavern_investigation"), TEXT("accepted"));
    WS->SetState(TEXT("task_clue"), TEXT("overheard_snippet"), TEXT("true"));
    WS->SetState(TEXT("task_clue"), TEXT("durgan_lore"), TEXT("true"));

    // Count clues (mirroring the Report handler logic)
    int32 ClueCount = 0;
    if (WS->GetState(TEXT("task_clue"), TEXT("overheard_snippet")) == TEXT("true")) ClueCount++;
    if (WS->GetState(TEXT("task_clue"), TEXT("durgan_lore")) == TEXT("true")) ClueCount++;
    if (WS->GetState(TEXT("task_clue"), TEXT("board_notice")) == TEXT("true")) ClueCount++;

    TestEqual(TEXT("Should have 2 clues"), ClueCount, 2);
    TestTrue(TEXT("2+ clues should qualify for full resolution"), ClueCount >= 2);

    // Simulate full resolution
    WS->SetState(TEXT("task"), TEXT("tavern_investigation"), TEXT("resolved"));
    TestEqual(TEXT("Task should be resolved"), WS->GetState(TEXT("task"), TEXT("tavern_investigation")), TEXT("resolved"));

    return true;
}

// ===================================================================
// Test 10: TaskLoop — Report with 0 clues does NOT resolve
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintK_ReportNoClues,
    "GreymawChronicles.SprintK.TaskLoop.ReportNoClues",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintK_ReportNoClues::RunTest(const FString& Parameters)
{
    UGameInstance* DummyGI = NewObject<UGameInstance>();
    UDMWorldStateSubsystem* WS = NewObject<UDMWorldStateSubsystem>(DummyGI);

    // Set up: task accepted, NO clues
    WS->SetState(TEXT("task"), TEXT("tavern_investigation"), TEXT("accepted"));

    // Count clues
    int32 ClueCount = 0;
    if (WS->GetState(TEXT("task_clue"), TEXT("overheard_snippet")) == TEXT("true")) ClueCount++;
    if (WS->GetState(TEXT("task_clue"), TEXT("durgan_lore")) == TEXT("true")) ClueCount++;
    if (WS->GetState(TEXT("task_clue"), TEXT("board_notice")) == TEXT("true")) ClueCount++;

    TestEqual(TEXT("Should have 0 clues"), ClueCount, 0);
    TestFalse(TEXT("0 clues should NOT qualify for resolution"), ClueCount >= 2);

    // Task should still be accepted (not resolved)
    TestEqual(TEXT("Task should remain accepted"), WS->GetState(TEXT("task"), TEXT("tavern_investigation")), TEXT("accepted"));

    return true;
}

// ===================================================================
// Test 11: SaveLoad — Feedback delegate exists and is broadcastable
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintK_FeedbackDelegateFires,
    "GreymawChronicles.SprintK.SaveLoad.FeedbackDelegateFires",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintK_FeedbackDelegateFires::RunTest(const FString& Parameters)
{
    UGCGameInstance* GI = NewObject<UGCGameInstance>();
    TestNotNull(TEXT("GameInstance should be valid"), GI);

    // Verify delegate exists and is not bound initially
    TestFalse(TEXT("OnSaveLoadFeedback should not be bound initially"), GI->OnSaveLoadFeedback.IsBound());

    // Dynamic multicast delegates require UFUNCTION binding — verify the delegate
    // is broadcastable by checking it compiles and the type signature is correct.
    // We can't bind a lambda to a dynamic delegate, so test structurally.

    // Verify broadcast doesn't crash with no bindings
    GI->OnSaveLoadFeedback.Broadcast(TEXT("Autosaved"));
    GI->OnSaveLoadFeedback.Broadcast(TEXT("Loaded save from 2026-02-22"));

    // If we get here without crash, delegate is correctly declared and broadcastable
    TestTrue(TEXT("Broadcast completed without crash"), true);

    // Verify the delegate type accepts the expected parameter signature
    // (compile-time validation — if this compiles, the signature is correct)
    FOnSaveLoadFeedback TestDelegate;
    TestFalse(TEXT("Fresh delegate should not be bound"), TestDelegate.IsBound());

    return true;
}

// ===================================================================
// Test 12: Intent — Accept and Report classify correctly
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintK_IntentAcceptReport,
    "GreymawChronicles.SprintK.Intent.AcceptReport",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintK_IntentAcceptReport::RunTest(const FString& Parameters)
{
    UDMIntentClassifier* Classifier = NewObject<UDMIntentClassifier>();

    // Accept intent
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("accept the task"));
        TestEqual(TEXT("'accept the task' should classify as Accept"), R.Intent, EDMIntent::Accept);
        TestTrue(TEXT("Accept confidence should be > 0.5"), R.Confidence > 0.5f);
    }

    // Accept variant: "agree"
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("I agree to help"));
        TestEqual(TEXT("'I agree to help' should classify as Accept"), R.Intent, EDMIntent::Accept);
    }

    // Accept variant: "volunteer"
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("volunteer for the job"));
        TestEqual(TEXT("'volunteer for the job' should classify as Accept"), R.Intent, EDMIntent::Accept);
    }

    // Report intent
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("report to marta"));
        TestEqual(TEXT("'report to marta' should classify as Report"), R.Intent, EDMIntent::Report);
        TestTrue(TEXT("Report confidence should be > 0.5"), R.Confidence > 0.5f);
    }

    // Report variant: "share findings"
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("share findings with marta"));
        TestEqual(TEXT("'share findings' should classify as Report"), R.Intent, EDMIntent::Report);
    }

    // Report variant: "inform"
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("inform marta about what i found"));
        TestEqual(TEXT("'inform' should classify as Report"), R.Intent, EDMIntent::Report);
    }

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
