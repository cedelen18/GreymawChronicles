#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Combat/GCEnemyTemplate.h"
#include "Combat/GCCombatEncounter.h"
#include "Quest/GCQuestSubsystem.h"
#include "Audio/GCAudioSubsystem.h"
#include "Core/GreymawSaveGame.h"
#include "DungeonMaster/DMDialogueTree.h"
#include "DungeonMaster/DMNarrationPool.h"
#include "DungeonMaster/DMWorldStateSubsystem.h"
#include "Gameplay/GCCharacterSheet.h"
#include "Gameplay/DiceRoller.h"
#include "Engine/GameInstance.h"

/**
 * Sprint L Tests
 *
 * 15 tests covering Combat Encounter, Quest Journal, Scene Transition,
 * Multi-Slot Save, Audio Framework, and Dialogue Tree systems.
 */

// ===================================================================
// Test 1: Combat.EnemyTemplateDefaults
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintL_Combat_EnemyTemplateDefaults,
    "GreymawChronicles.SprintL.Combat.EnemyTemplateDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintL_Combat_EnemyTemplateDefaults::RunTest(const FString& Parameters)
{
    FGCEnemyTemplate Goblin = FGCEnemyTemplate::MakeGoblin();

    TestEqual(TEXT("Goblin name"), Goblin.CreatureName, TEXT("Goblin"));
    TestEqual(TEXT("Goblin MaxHP"), Goblin.MaxHP, 7);
    TestEqual(TEXT("Goblin CurrentHP"), Goblin.CurrentHP, 7);
    TestEqual(TEXT("Goblin AC"), Goblin.ArmorClass, 15);
    TestEqual(TEXT("Goblin AttackModifier"), Goblin.AttackModifier, 4);
    TestEqual(TEXT("Goblin InitiativeModifier"), Goblin.InitiativeModifier, 2);
    TestEqual(TEXT("Goblin Weapon Name"), Goblin.Weapon.Name, TEXT("Scimitar"));
    TestEqual(TEXT("Goblin Weapon DamageDiceCount"), Goblin.Weapon.DamageDiceCount, 1);
    TestEqual(TEXT("Goblin Weapon DamageDieSize"), Goblin.Weapon.DamageDieSize, 6);

    return true;
}

// ===================================================================
// Test 2: Combat.InitiativeOrder
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintL_Combat_InitiativeOrder,
    "GreymawChronicles.SprintL.Combat.InitiativeOrder",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintL_Combat_InitiativeOrder::RunTest(const FString& Parameters)
{
    UGCCharacterSheet* Player = NewObject<UGCCharacterSheet>();
    Player->AbilityScores.Add(EGCAbility::Dexterity, 14);  // +2 modifier

    TArray<FGCEnemyTemplate> Enemies;
    Enemies.Add(FGCEnemyTemplate::MakeGoblin());
    Enemies.Add(FGCEnemyTemplate::MakeGoblin());

    UDiceRoller* Dice = NewObject<UDiceRoller>();
    UGCCombatEncounter* Encounter = NewObject<UGCCombatEncounter>();
    Encounter->InitializeEncounter(Player, Enemies, Dice);
    Encounter->RollInitiative();

    // Should have 3 entries: Player + 2 goblins
    TestEqual(TEXT("TurnOrder should have 3 entries"), Encounter->TurnOrder.Num(), 3);

    // Verify Player is in the list
    bool bFoundPlayer = false;
    for (const FString& Entry : Encounter->TurnOrder)
    {
        if (Entry == TEXT("Player")) { bFoundPlayer = true; break; }
    }
    TestTrue(TEXT("Player should be in TurnOrder"), bFoundPlayer);

    return true;
}

// ===================================================================
// Test 3: Combat.TurnAdvancement
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintL_Combat_TurnAdvancement,
    "GreymawChronicles.SprintL.Combat.TurnAdvancement",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintL_Combat_TurnAdvancement::RunTest(const FString& Parameters)
{
    UGCCharacterSheet* Player = NewObject<UGCCharacterSheet>();
    Player->CurrentHP = 20;
    Player->MaxHP = 20;

    TArray<FGCEnemyTemplate> Enemies;
    Enemies.Add(FGCEnemyTemplate::MakeGoblin());

    UDiceRoller* Dice = NewObject<UDiceRoller>();
    UGCCombatEncounter* Encounter = NewObject<UGCCombatEncounter>();
    Encounter->InitializeEncounter(Player, Enemies, Dice);
    Encounter->RollInitiative();

    TestEqual(TEXT("Should have 2 entries"), Encounter->TurnOrder.Num(), 2);
    TestEqual(TEXT("Round starts at 1"), Encounter->GetRoundNumber(), 1);
    TestEqual(TEXT("CurrentTurnIndex starts at 0"), Encounter->CurrentTurnIndex, 0);

    // Advance through one full round
    Encounter->AdvanceTurn();
    TestEqual(TEXT("CurrentTurnIndex should be 1"), Encounter->CurrentTurnIndex, 1);

    // Advance wraps around
    Encounter->AdvanceTurn();
    TestEqual(TEXT("CurrentTurnIndex wraps to 0"), Encounter->CurrentTurnIndex, 0);
    TestEqual(TEXT("Round incremented to 2"), Encounter->GetRoundNumber(), 2);

    return true;
}

// ===================================================================
// Test 4: Combat.VictoryCondition
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintL_Combat_VictoryCondition,
    "GreymawChronicles.SprintL.Combat.VictoryCondition",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintL_Combat_VictoryCondition::RunTest(const FString& Parameters)
{
    UGCCharacterSheet* Player = NewObject<UGCCharacterSheet>();
    Player->CurrentHP = 20;
    Player->MaxHP = 20;

    TArray<FGCEnemyTemplate> Enemies;
    FGCEnemyTemplate Goblin = FGCEnemyTemplate::MakeGoblin();
    Enemies.Add(Goblin);

    UDiceRoller* Dice = NewObject<UDiceRoller>();
    UGCCombatEncounter* Encounter = NewObject<UGCCombatEncounter>();
    Encounter->InitializeEncounter(Player, Enemies, Dice);

    TestEqual(TEXT("Outcome starts as Ongoing"), Encounter->GetOutcome(), ECombatOutcome::Ongoing);
    TestFalse(TEXT("Combat not over at start"), Encounter->IsCombatOver());

    // Kill all enemies
    Encounter->GetEnemyRoster()[0].CurrentHP = 0;

    TestEqual(TEXT("Outcome is Victory"), Encounter->GetOutcome(), ECombatOutcome::Victory);
    TestTrue(TEXT("Combat is over"), Encounter->IsCombatOver());

    return true;
}

// ===================================================================
// Test 5: Combat.DefeatCondition
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintL_Combat_DefeatCondition,
    "GreymawChronicles.SprintL.Combat.DefeatCondition",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintL_Combat_DefeatCondition::RunTest(const FString& Parameters)
{
    UGCCharacterSheet* Player = NewObject<UGCCharacterSheet>();
    Player->CurrentHP = 10;
    Player->MaxHP = 20;

    TArray<FGCEnemyTemplate> Enemies;
    Enemies.Add(FGCEnemyTemplate::MakeGoblin());

    UDiceRoller* Dice = NewObject<UDiceRoller>();
    UGCCombatEncounter* Encounter = NewObject<UGCCombatEncounter>();
    Encounter->InitializeEncounter(Player, Enemies, Dice);

    TestEqual(TEXT("Outcome starts as Ongoing"), Encounter->GetOutcome(), ECombatOutcome::Ongoing);

    // Player HP drops to 0
    Player->CurrentHP = 0;

    TestEqual(TEXT("Outcome is Defeat"), Encounter->GetOutcome(), ECombatOutcome::Defeat);
    TestTrue(TEXT("Combat is over"), Encounter->IsCombatOver());

    return true;
}

// ===================================================================
// Test 6: Quest.StartAndQuery
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintL_Quest_StartAndQuery,
    "GreymawChronicles.SprintL.Quest.StartAndQuery",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintL_Quest_StartAndQuery::RunTest(const FString& Parameters)
{
    UGameInstance* DummyGI = NewObject<UGameInstance>();
    UGCQuestSubsystem* QuestSys = NewObject<UGCQuestSubsystem>(DummyGI);

    // Before starting, status should be NotStarted
    TestEqual(TEXT("Quest not started"), QuestSys->GetQuestStatus(TEXT("test_quest")), EGCQuestStatus::NotStarted);

    QuestSys->StartQuest(TEXT("test_quest"), TEXT("Test Quest"), TEXT("A test quest description."));

    TestEqual(TEXT("Quest is Active after starting"), QuestSys->GetQuestStatus(TEXT("test_quest")), EGCQuestStatus::Active);

    TArray<FGCQuestEntry> Active = QuestSys->GetActiveQuests();
    TestEqual(TEXT("One active quest"), Active.Num(), 1);
    TestEqual(TEXT("Active quest ID matches"), Active[0].QuestId, TEXT("test_quest"));

    return true;
}

// ===================================================================
// Test 7: Quest.UpdateObjective
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintL_Quest_UpdateObjective,
    "GreymawChronicles.SprintL.Quest.UpdateObjective",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintL_Quest_UpdateObjective::RunTest(const FString& Parameters)
{
    UGameInstance* DummyGI = NewObject<UGameInstance>();
    UGCQuestSubsystem* QuestSys = NewObject<UGCQuestSubsystem>(DummyGI);
    QuestSys->StartQuest(TEXT("test_quest"), TEXT("Test Quest"), TEXT("Description."));
    QuestSys->UpdateObjective(TEXT("test_quest"), TEXT("Find the goblin cave."));

    const FGCQuestEntry* Entry = QuestSys->FindQuest(TEXT("test_quest"));
    TestNotNull(TEXT("Quest should exist"), Entry);
    if (Entry)
    {
        TestEqual(TEXT("Objective updated"), Entry->CurrentObjective, TEXT("Find the goblin cave."));
    }

    return true;
}

// ===================================================================
// Test 8: Quest.CompleteQuest
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintL_Quest_CompleteQuest,
    "GreymawChronicles.SprintL.Quest.CompleteQuest",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintL_Quest_CompleteQuest::RunTest(const FString& Parameters)
{
    UGameInstance* DummyGI = NewObject<UGameInstance>();
    UGCQuestSubsystem* QuestSys = NewObject<UGCQuestSubsystem>(DummyGI);
    QuestSys->StartQuest(TEXT("test_quest"), TEXT("Test Quest"), TEXT("Description."));
    QuestSys->CompleteQuest(TEXT("test_quest"));

    TestEqual(TEXT("Quest is Completed"), QuestSys->GetQuestStatus(TEXT("test_quest")), EGCQuestStatus::Completed);

    TArray<FGCQuestEntry> Active = QuestSys->GetActiveQuests();
    TestEqual(TEXT("No active quests after completion"), Active.Num(), 0);

    return true;
}

// ===================================================================
// Test 9: Scene.TrailNarrationExists
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintL_Scene_TrailNarrationExists,
    "GreymawChronicles.SprintL.Scene.TrailNarrationExists",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintL_Scene_TrailNarrationExists::RunTest(const FString& Parameters)
{
    UDMNarrationPool* Pool = NewObject<UDMNarrationPool>();
    Pool->PopulateTrailDefaults();

    const FString TrailArrive = Pool->PickRandom(TEXT("trail_arrive"));
    TestFalse(TEXT("trail_arrive should return non-empty"), TrailArrive.IsEmpty());

    const FString TrailLook = Pool->PickRandom(TEXT("trail_look"));
    TestFalse(TEXT("trail_look should return non-empty"), TrailLook.IsEmpty());

    const FString TrailCave = Pool->PickRandom(TEXT("trail_cave_entrance"));
    TestFalse(TEXT("trail_cave_entrance should return non-empty"), TrailCave.IsEmpty());

    const FString TrailAmbient = Pool->PickRandom(TEXT("trail_ambient"));
    TestFalse(TEXT("trail_ambient should return non-empty"), TrailAmbient.IsEmpty());

    return true;
}

// ===================================================================
// Test 10: Scene.SceneIdTracking
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintL_Scene_SceneIdTracking,
    "GreymawChronicles.SprintL.Scene.SceneIdTracking",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintL_Scene_SceneIdTracking::RunTest(const FString& Parameters)
{
    UGameInstance* DummyGI = NewObject<UGameInstance>();
    UDMWorldStateSubsystem* WorldState = NewObject<UDMWorldStateSubsystem>(DummyGI);
    WorldState->SetState(TEXT("scene"), TEXT("current"), TEXT("greymaw_trail"));

    const FString SceneId = WorldState->GetState(TEXT("scene"), TEXT("current"));
    TestEqual(TEXT("Scene ID round-trip"), SceneId, TEXT("greymaw_trail"));

    return true;
}

// ===================================================================
// Test 11: Save.MultiSlotNaming
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintL_Save_MultiSlotNaming,
    "GreymawChronicles.SprintL.Save.MultiSlotNaming",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintL_Save_MultiSlotNaming::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("Slot 0 name"), UGreymawSaveGame::MakeSlotName(0), TEXT("GreymawSlot_0"));
    TestEqual(TEXT("Slot 1 name"), UGreymawSaveGame::MakeSlotName(1), TEXT("GreymawSlot_1"));
    TestEqual(TEXT("Slot 2 name"), UGreymawSaveGame::MakeSlotName(2), TEXT("GreymawSlot_2"));

    // Clamped values
    TestEqual(TEXT("Slot -1 clamps to 0"), UGreymawSaveGame::MakeSlotName(-1), TEXT("GreymawSlot_0"));
    TestEqual(TEXT("Slot 99 clamps to 2"), UGreymawSaveGame::MakeSlotName(99), TEXT("GreymawSlot_2"));

    return true;
}

// ===================================================================
// Test 12: Save.QuestLogPersistence
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintL_Save_QuestLogPersistence,
    "GreymawChronicles.SprintL.Save.QuestLogPersistence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintL_Save_QuestLogPersistence::RunTest(const FString& Parameters)
{
    UGameInstance* DummyGI = NewObject<UGameInstance>();

    // Create quest subsystem, add quest, serialize
    UGCQuestSubsystem* QuestSys = NewObject<UGCQuestSubsystem>(DummyGI);
    QuestSys->StartQuest(TEXT("persist_test"), TEXT("Persistence Test"), TEXT("Testing save/load."));
    QuestSys->UpdateObjective(TEXT("persist_test"), TEXT("Verify data survives."));

    const FString JSON = QuestSys->ToJSON();
    TestFalse(TEXT("JSON should not be empty"), JSON.IsEmpty());

    // Create new subsystem and restore
    UGCQuestSubsystem* Restored = NewObject<UGCQuestSubsystem>(DummyGI);
    const bool bLoaded = Restored->FromJSON(JSON);
    TestTrue(TEXT("FromJSON should succeed"), bLoaded);

    TestEqual(TEXT("Restored quest status"), Restored->GetQuestStatus(TEXT("persist_test")), EGCQuestStatus::Active);

    const FGCQuestEntry* Entry = Restored->FindQuest(TEXT("persist_test"));
    TestNotNull(TEXT("Restored quest should exist"), Entry);
    if (Entry)
    {
        TestEqual(TEXT("Restored title"), Entry->Title, TEXT("Persistence Test"));
        TestEqual(TEXT("Restored objective"), Entry->CurrentObjective, TEXT("Verify data survives."));
    }

    return true;
}

// ===================================================================
// Test 13: Audio.NullSafePlayback
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintL_Audio_NullSafePlayback,
    "GreymawChronicles.SprintL.Audio.NullSafePlayback",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintL_Audio_NullSafePlayback::RunTest(const FString& Parameters)
{
    UGameInstance* DummyGI = NewObject<UGameInstance>();
    UGCAudioSubsystem* Audio = NewObject<UGCAudioSubsystem>(DummyGI);

    // Playing an event with no registered sound should not crash
    Audio->PlayEvent(EGCAudioEvent::Combat_Hit);
    Audio->PlayEvent(EGCAudioEvent::UI_ButtonClick);
    Audio->StopEvent(EGCAudioEvent::Music_TavernTheme);

    // Volume control should work
    Audio->SetMasterVolume(0.5f);
    TestEqual(TEXT("Master volume set"), Audio->GetMasterVolume(), 0.5f);

    // Register nullptr sound — should not crash
    Audio->RegisterSoundForEvent(EGCAudioEvent::Combat_Hit, nullptr);
    Audio->PlayEvent(EGCAudioEvent::Combat_Hit);

    return true;
}

// ===================================================================
// Test 14: Dialogue.OptionFiltering
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintL_Dialogue_OptionFiltering,
    "GreymawChronicles.SprintL.Dialogue.OptionFiltering",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintL_Dialogue_OptionFiltering::RunTest(const FString& Parameters)
{
    UDMDialogueTree* Tree = NewObject<UDMDialogueTree>();
    Tree->PopulateMartaDialogue();

    // Without WorldState (nullptr) — conditional options should be filtered out
    TArray<FGCDialogueOption> OptionsNoState = Tree->GetAvailableOptions(TEXT("marta_root"), nullptr);

    // Marta root has 4 options, one requires task:tavern_investigation=accepted
    // Without state: should get 3 options (the unconditional ones)
    TestEqual(TEXT("Options without state should be 3"), OptionsNoState.Num(), 3);

    // With WorldState that has the required state
    UGameInstance* DummyGI = NewObject<UGameInstance>();
    UDMWorldStateSubsystem* WorldState = NewObject<UDMWorldStateSubsystem>(DummyGI);
    WorldState->SetState(TEXT("task"), TEXT("tavern_investigation"), TEXT("accepted"));

    TArray<FGCDialogueOption> OptionsWithState = Tree->GetAvailableOptions(TEXT("marta_root"), WorldState);
    TestEqual(TEXT("Options with state should be 4"), OptionsWithState.Num(), 4);

    return true;
}

// ===================================================================
// Test 15: Dialogue.NodeChaining
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintL_Dialogue_NodeChaining,
    "GreymawChronicles.SprintL.Dialogue.NodeChaining",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintL_Dialogue_NodeChaining::RunTest(const FString& Parameters)
{
    UDMDialogueTree* Tree = NewObject<UDMDialogueTree>();

    // Register two linked nodes
    FGCDialogueNode NodeA;
    NodeA.NodeId = TEXT("node_a");
    NodeA.NPCGreeting = TEXT("Hello, adventurer.");

    FGCDialogueOption OptionToB;
    OptionToB.OptionText = TEXT("Tell me more.");
    OptionToB.ResponseNarration = TEXT("Very well...");
    OptionToB.NextNodeId = TEXT("node_b");
    NodeA.Options.Add(OptionToB);

    FGCDialogueOption OptionEnd;
    OptionEnd.OptionText = TEXT("Goodbye.");
    OptionEnd.ResponseNarration = TEXT("Farewell.");
    OptionEnd.NextNodeId = TEXT("");  // end conversation
    NodeA.Options.Add(OptionEnd);

    Tree->RegisterNode(NodeA);

    FGCDialogueNode NodeB;
    NodeB.NodeId = TEXT("node_b");
    NodeB.NPCGreeting = TEXT("This goes deeper...");
    Tree->RegisterNode(NodeB);

    // Verify node A exists and has options
    TestTrue(TEXT("Node A exists"), Tree->HasNode(TEXT("node_a")));
    TestTrue(TEXT("Node B exists"), Tree->HasNode(TEXT("node_b")));

    TArray<FGCDialogueOption> Options = Tree->GetAvailableOptions(TEXT("node_a"), nullptr);
    TestEqual(TEXT("Node A has 2 options"), Options.Num(), 2);

    // Select option 0 (Tell me more) — NextNodeId should be node_b
    TestEqual(TEXT("Option 0 chains to node_b"), Options[0].NextNodeId, TEXT("node_b"));

    // Verify we can get node_b
    FGCDialogueNode RetrievedB = Tree->GetNode(TEXT("node_b"));
    TestEqual(TEXT("Node B greeting"), RetrievedB.NPCGreeting, TEXT("This goes deeper..."));

    // Option 1 ends conversation (empty NextNodeId)
    TestTrue(TEXT("Option 1 ends conversation"), Options[1].NextNodeId.IsEmpty());

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
