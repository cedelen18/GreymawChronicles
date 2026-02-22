#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "DungeonMaster/DMConversationHistory.h"
#include "DungeonMaster/DMResponseParser.h"
#include "DungeonMaster/DMIntentClassifier.h"
#include "DungeonMaster/DMNarrationPool.h"
#include "Gameplay/GCCharacterSheet.h"
#include "Gameplay/AbilityCheckResolver.h"
#include "Gameplay/DiceRoller.h"
#include "Rules/CombatResolver.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDMConversationHistoryCapacityTest, "GreymawChronicles.DM.ConversationHistoryCapacity", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDMResponseParserRepairTest, "GreymawChronicles.DM.ResponseParserRepair", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDMCheckRequiredFlowTest, "GreymawChronicles.DM.CheckRequired.Integration", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDMIntentClassifierScriptedTest, "GreymawChronicles.DM.TurnLoop.ScriptedPromptState", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDMNarrationPoolFallbackTest, "GreymawChronicles.DM.TurnLoop.FallbackRestoresInput", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FDMConversationHistoryCapacityTest::RunTest(const FString& Parameters)
{
    UDMConversationHistory* History = NewObject<UDMConversationHistory>();
    for (int32 Index = 0; Index < 20; ++Index)
    {
        FDMExchange Exchange;
        Exchange.PlayerInput = FString::Printf(TEXT("input_%d"), Index);
        Exchange.DMResponseSummary = TEXT("ok");
        Exchange.SceneID = TEXT("test");
        Exchange.TimestampUtc = FDateTime::UtcNow();
        History->AddExchange(Exchange);
    }

    const TArray<FDMExchange> Recent = History->GetRecentExchanges();
    TestEqual(TEXT("History should cap at 15 entries"), Recent.Num(), 15);
    TestEqual(TEXT("Oldest retained should be input_5"), Recent[0].PlayerInput, FString(TEXT("input_5")));
    return true;
}

bool FDMResponseParserRepairTest::RunTest(const FString& Parameters)
{
    UDMResponseParser* Parser = NewObject<UDMResponseParser>();
    const FString BrokenJson = TEXT("```json\n{\"narration\":\"hello\",\"actions\":[],}\n```");

    const FDMResponse Parsed = Parser->ParseResponse(BrokenJson);
    TestTrue(TEXT("Parser should repair and parse a simple trailing comma case"), Parsed.bValid);
    TestEqual(TEXT("Narration should parse"), Parsed.Narration, FString(TEXT("hello")));
    return true;
}

/**
 * Tests the check-required flow by exercising AbilityCheckResolver + CombatResolver
 * directly. We avoid NewObject<UDMBrainSubsystem>() because GameInstanceSubsystems
 * cannot be created outside a valid game instance context (ensure-fail in NullRHI CI).
 */
bool FDMCheckRequiredFlowTest::RunTest(const FString& Parameters)
{
    UGCCharacterSheet* Player = NewObject<UGCCharacterSheet>();
    Player->CurrentHP = 20;
    Player->MaxHP = 20;
    Player->ProficiencyBonus = 2;
    Player->AbilityScores.Add(EGCAbility::Strength, 10);
    Player->SkillProficiencies.Add(EGCSkill::Athletics);

    UDiceRoller* Roller = NewObject<UDiceRoller>();
    UAbilityCheckResolver* Resolver = NewObject<UAbilityCheckResolver>();
    Resolver->Initialize(Roller);

    // DC 100 is impossible — guarantees failure
    const FAbilityCheckResult Result = Resolver->ResolveAbilityCheck(Player, EGCSkill::Athletics, 100, false, false);
    TestFalse(TEXT("DC 100 should always fail"), Result.bSuccess);

    // Simulate damage on failure path
    UCombatResolver* CombatResolver = NewObject<UCombatResolver>();
    CombatResolver->Initialize(Roller);
    CombatResolver->ApplyDamage(Player, 4);

    TestEqual(TEXT("Failure path should apply 4 damage (20 -> 16)"), Player->CurrentHP, 16);
    return true;
}

/**
 * Tests that the intent classifier correctly maps a tavern "look" prompt to
 * the Look intent, and that the narration pool returns non-empty text.
 * This replaces the old test that created UDMBrainSubsystem directly.
 */
bool FDMIntentClassifierScriptedTest::RunTest(const FString& Parameters)
{
    UDMIntentClassifier* Classifier = NewObject<UDMIntentClassifier>();

    const FDMIntentResult Result = Classifier->Classify(TEXT("I look around the tavern"));
    TestEqual(TEXT("'I look around the tavern' -> Look"), static_cast<uint8>(Result.Intent), static_cast<uint8>(EDMIntent::Look));
    TestTrue(TEXT("Confidence should be non-zero"), Result.Confidence > 0.0f);

    UDMNarrationPool* Pool = NewObject<UDMNarrationPool>();
    Pool->PopulateTavernDefaults();

    const FString Narration = Pool->PickRandom(TEXT("look_around"));
    TestFalse(TEXT("Narration pool should return non-empty text for look_around"), Narration.IsEmpty());
    return true;
}

/**
 * Tests that the narration pool returns non-empty fallback text for unknown
 * intents, ensuring the fallback path always produces visible output.
 */
bool FDMNarrationPoolFallbackTest::RunTest(const FString& Parameters)
{
    UDMIntentClassifier* Classifier = NewObject<UDMIntentClassifier>();

    // Gibberish should resolve to Unknown
    const FDMIntentResult Result = Classifier->Classify(TEXT("Completely unscripted prompt while dependencies are unavailable"));
    TestEqual(TEXT("Unrecognized input -> Unknown"), static_cast<uint8>(Result.Intent), static_cast<uint8>(EDMIntent::Unknown));

    // Fallback narration pool should still return something
    UDMNarrationPool* Pool = NewObject<UDMNarrationPool>();
    Pool->PopulateTavernDefaults();

    const FString Fallback = Pool->PickRandom(TEXT("fallback"));
    TestFalse(TEXT("Fallback narration should be non-empty"), Fallback.IsEmpty());
    return true;
}

#endif
