#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Core/GreymawSaveGame.h"
#include "DungeonMaster/DMBrainSubsystem.h"
#include "DungeonMaster/DMConversationHistory.h"
#include "DungeonMaster/DMIntentClassifier.h"
#include "DungeonMaster/DMWorldStateSubsystem.h"
#include "Gameplay/GCCharacterSheet.h"
#include "Engine/GameInstance.h"

/**
 * Sprint J Tests
 *
 * 10 tests covering WorldState serialization, inventory tracking,
 * save/load objects, NPC name consistency, and error handling.
 */

// ===================================================================
// Test 1: WorldState ToJSON / FromJSON round-trip
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintJ_WorldStateSerialization,
    "GreymawChronicles.SprintJ.WorldState.Serialization",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintJ_WorldStateSerialization::RunTest(const FString& Parameters)
{
    UGameInstance* DummyGI = NewObject<UGameInstance>();
    UDMWorldStateSubsystem* WS1 = NewObject<UDMWorldStateSubsystem>(DummyGI);

    WS1->SetState(TEXT("npc_disposition"), TEXT("marta"), TEXT("friendly"));
    WS1->SetState(TEXT("npc_disposition"), TEXT("durgan"), TEXT("open"));
    WS1->SetState(TEXT("world_flags"), TEXT("coin_found"), TEXT("true"));

    const FString JSON = WS1->ToJSON();
    TestFalse(TEXT("JSON should not be empty"), JSON.IsEmpty());

    UDMWorldStateSubsystem* WS2 = NewObject<UDMWorldStateSubsystem>(DummyGI);
    const bool bParsed = WS2->FromJSON(JSON);
    TestTrue(TEXT("FromJSON should succeed"), bParsed);

    TestEqual(TEXT("marta disposition"), WS2->GetState(TEXT("npc_disposition"), TEXT("marta")), TEXT("friendly"));
    TestEqual(TEXT("durgan disposition"), WS2->GetState(TEXT("npc_disposition"), TEXT("durgan")), TEXT("open"));
    TestEqual(TEXT("coin_found flag"), WS2->GetState(TEXT("world_flags"), TEXT("coin_found")), TEXT("true"));

    return true;
}

// ===================================================================
// Test 2: WorldState ClearAll
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintJ_WorldStateClearAll,
    "GreymawChronicles.SprintJ.WorldState.ClearAll",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintJ_WorldStateClearAll::RunTest(const FString& Parameters)
{
    UGameInstance* DummyGI = NewObject<UGameInstance>();
    UDMWorldStateSubsystem* WS = NewObject<UDMWorldStateSubsystem>(DummyGI);

    WS->SetState(TEXT("cat1"), TEXT("key1"), TEXT("val1"));
    WS->SetState(TEXT("cat2"), TEXT("key2"), TEXT("val2"));
    TestTrue(TEXT("key1 should exist before clear"), WS->HasState(TEXT("cat1"), TEXT("key1")));

    WS->ClearAll();
    TestFalse(TEXT("key1 should not exist after clear"), WS->HasState(TEXT("cat1"), TEXT("key1")));
    TestFalse(TEXT("key2 should not exist after clear"), WS->HasState(TEXT("cat2"), TEXT("key2")));

    return true;
}

// ===================================================================
// Test 3: Inventory AddUnique behavior
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintJ_InventoryAddItem,
    "GreymawChronicles.SprintJ.Inventory.AddItem",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintJ_InventoryAddItem::RunTest(const FString& Parameters)
{
    UGCCharacterSheet* Sheet = NewObject<UGCCharacterSheet>();
    TestEqual(TEXT("Equipment should start empty"), Sheet->Equipment.Num(), 0);

    Sheet->Equipment.AddUnique(TEXT("mysterious_coin"));
    TestEqual(TEXT("Should have 1 item"), Sheet->Equipment.Num(), 1);
    TestEqual(TEXT("Item should be mysterious_coin"), Sheet->Equipment[0], TEXT("mysterious_coin"));

    // Duplicate add should not increase count
    Sheet->Equipment.AddUnique(TEXT("mysterious_coin"));
    TestEqual(TEXT("Should still have 1 item after duplicate"), Sheet->Equipment.Num(), 1);

    // Different item should increase count
    Sheet->Equipment.AddUnique(TEXT("stolen_coin_pouch"));
    TestEqual(TEXT("Should have 2 items"), Sheet->Equipment.Num(), 2);

    return true;
}

// ===================================================================
// Test 4: CharacterSheet Equipment survives JSON round-trip
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintJ_InventoryJSONRoundTrip,
    "GreymawChronicles.SprintJ.Inventory.JSONRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintJ_InventoryJSONRoundTrip::RunTest(const FString& Parameters)
{
    UGCCharacterSheet* Sheet1 = NewObject<UGCCharacterSheet>();
    Sheet1->CharacterName = TEXT("TestHero");
    Sheet1->Level = 3;
    Sheet1->MaxHP = 25;
    Sheet1->CurrentHP = 20;
    Sheet1->Equipment.Add(TEXT("mysterious_coin"));
    Sheet1->Equipment.Add(TEXT("stolen_coin_pouch"));
    Sheet1->Equipment.Add(TEXT("rusty_dagger"));

    const FString JSON = Sheet1->ToCompactJSON();
    TestFalse(TEXT("JSON should not be empty"), JSON.IsEmpty());

    UGCCharacterSheet* Sheet2 = NewObject<UGCCharacterSheet>();
    FString Error;
    const bool bParsed = Sheet2->FromJSON(JSON, Error);
    TestTrue(TEXT("FromJSON should succeed"), bParsed);

    TestEqual(TEXT("Equipment count should match"), Sheet2->Equipment.Num(), 3);
    TestTrue(TEXT("Should contain mysterious_coin"), Sheet2->Equipment.Contains(TEXT("mysterious_coin")));
    TestTrue(TEXT("Should contain stolen_coin_pouch"), Sheet2->Equipment.Contains(TEXT("stolen_coin_pouch")));
    TestTrue(TEXT("Should contain rusty_dagger"), Sheet2->Equipment.Contains(TEXT("rusty_dagger")));

    return true;
}

// ===================================================================
// Test 5: SaveGame object creation and fields
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintJ_SaveLoadObjectCreation,
    "GreymawChronicles.SprintJ.SaveLoad.ObjectCreation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintJ_SaveLoadObjectCreation::RunTest(const FString& Parameters)
{
    UGreymawSaveGame* SaveObj = NewObject<UGreymawSaveGame>();
    TestNotNull(TEXT("SaveGame object should be valid"), SaveObj);

    SaveObj->CharacterSheetJSON = TEXT("{\"name\":\"test\"}");
    SaveObj->WorldStateJSON = TEXT("{\"npc_disposition\":{}}");
    SaveObj->ConversationHistoryJSON = TEXT("{\"exchanges\":[]}");
    SaveObj->SaveTimestamp = TEXT("2026-02-22T00:00:00Z");

    TestFalse(TEXT("CharacterSheetJSON should not be empty"), SaveObj->CharacterSheetJSON.IsEmpty());
    TestFalse(TEXT("WorldStateJSON should not be empty"), SaveObj->WorldStateJSON.IsEmpty());
    TestFalse(TEXT("ConversationHistoryJSON should not be empty"), SaveObj->ConversationHistoryJSON.IsEmpty());
    TestFalse(TEXT("SaveSlotName should not be empty"), UGreymawSaveGame::SaveSlotName.IsEmpty());

    return true;
}

// ===================================================================
// Test 6: ConversationHistory serialization round-trip
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintJ_ConversationRoundTrip,
    "GreymawChronicles.SprintJ.SaveLoad.ConversationRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintJ_ConversationRoundTrip::RunTest(const FString& Parameters)
{
    UDMConversationHistory* History1 = NewObject<UDMConversationHistory>();

    FDMExchange Ex1;
    Ex1.PlayerInput = TEXT("look around");
    Ex1.DMResponseSummary = TEXT("You see a tavern.");
    Ex1.SceneID = TEXT("tavern");
    Ex1.TimestampUtc = FDateTime::UtcNow();
    History1->AddExchange(Ex1);

    FDMExchange Ex2;
    Ex2.PlayerInput = TEXT("talk to marta");
    Ex2.DMResponseSummary = TEXT("Marta greets you.");
    Ex2.SceneID = TEXT("tavern");
    Ex2.TimestampUtc = FDateTime::UtcNow();
    History1->AddExchange(Ex2);

    const FString JSON = History1->SerializeToJSON();
    TestFalse(TEXT("JSON should not be empty"), JSON.IsEmpty());

    UDMConversationHistory* History2 = NewObject<UDMConversationHistory>();
    const bool bParsed = History2->DeserializeFromJSON(JSON);
    TestTrue(TEXT("DeserializeFromJSON should succeed"), bParsed);

    const TArray<FDMExchange> Exchanges = History2->GetRecentExchanges();
    TestEqual(TEXT("Should have 2 exchanges"), Exchanges.Num(), 2);

    return true;
}

// ===================================================================
// Test 7: NPC tags — "talk to marta" extracts subject
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintJ_NPCTagsRegistered,
    "GreymawChronicles.SprintJ.NPCSpawn.TagsRegistered",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintJ_NPCTagsRegistered::RunTest(const FString& Parameters)
{
    UDMIntentClassifier* Classifier = NewObject<UDMIntentClassifier>();

    const FDMIntentResult Result = Classifier->Classify(TEXT("talk to marta"));
    TestEqual(TEXT("Intent should be Talk"), Result.Intent, EDMIntent::Talk);
    TestTrue(TEXT("Subject should contain marta"), Result.Subject.Contains(TEXT("marta")));

    return true;
}

// ===================================================================
// Test 8: All 3 NPC names resolve via intent classifier
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintJ_NPCNameConsistency,
    "GreymawChronicles.SprintJ.NPCSpawn.NameConsistency",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintJ_NPCNameConsistency::RunTest(const FString& Parameters)
{
    UDMIntentClassifier* Classifier = NewObject<UDMIntentClassifier>();

    // Marta
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("talk to marta"));
        TestTrue(TEXT("marta subject extracted"), R.Subject.Contains(TEXT("marta")));
    }

    // Durgan
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("talk to durgan"));
        TestTrue(TEXT("durgan subject extracted"), R.Subject.Contains(TEXT("durgan")));
    }

    // Kael
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("talk to kael"));
        TestTrue(TEXT("kael subject extracted"), R.Subject.Contains(TEXT("kael")));
    }

    return true;
}

// ===================================================================
// Test 9: OnInventoryChanged delegate exists and can be bound
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintJ_InventoryDelegateExists,
    "GreymawChronicles.SprintJ.Inventory.DelegateExists",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintJ_InventoryDelegateExists::RunTest(const FString& Parameters)
{
    UGameInstance* DummyGI = NewObject<UGameInstance>();
    UDMBrainSubsystem* Brain = NewObject<UDMBrainSubsystem>(DummyGI);
    TestNotNull(TEXT("DMBrainSubsystem should be valid"), Brain);

    // Verify the delegate exists and is broadcastable (compile-time validation)
    TestTrue(TEXT("OnInventoryChanged should not be bound initially"),
        !Brain->OnInventoryChanged.IsBound());

    return true;
}

// ===================================================================
// Test 10: WorldState FromJSON handles invalid input gracefully
// ===================================================================
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSprintJ_WorldStateInvalidJSON,
    "GreymawChronicles.SprintJ.WorldState.InvalidJSON",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSprintJ_WorldStateInvalidJSON::RunTest(const FString& Parameters)
{
    UGameInstance* DummyGI = NewObject<UGameInstance>();
    UDMWorldStateSubsystem* WS = NewObject<UDMWorldStateSubsystem>(DummyGI);

    // Set some initial state
    WS->SetState(TEXT("cat"), TEXT("key"), TEXT("val"));
    TestTrue(TEXT("State should exist before bad parse"), WS->HasState(TEXT("cat"), TEXT("key")));

    // Attempt to parse malformed JSON
    const bool bResult = WS->FromJSON(TEXT("{this is not valid json!!!}"));
    TestFalse(TEXT("FromJSON should return false for invalid JSON"), bResult);

    // FromJSON calls ClearAll first, so previous state should be gone
    TestFalse(TEXT("State should be cleared after failed parse"), WS->HasState(TEXT("cat"), TEXT("key")));

    // Empty string should succeed (valid: no state to restore)
    const bool bEmpty = WS->FromJSON(TEXT(""));
    TestTrue(TEXT("FromJSON with empty string should succeed"), bEmpty);

    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
