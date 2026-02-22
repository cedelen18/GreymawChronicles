#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "DungeonMaster/DMConversationHistory.h"
#include "DungeonMaster/DMResponseParser.h"
#include "DungeonMaster/DMBrainSubsystem.h"
#include "Gameplay/GCCharacterSheet.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDMConversationHistoryCapacityTest, "GreymawChronicles.DM.ConversationHistoryCapacity", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDMResponseParserRepairTest, "GreymawChronicles.DM.ResponseParserRepair", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDMBrainCheckRequiredIntegrationTest, "GreymawChronicles.DM.CheckRequired.Integration", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

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

bool FDMBrainCheckRequiredIntegrationTest::RunTest(const FString& Parameters)
{
    UDMBrainSubsystem* Brain = NewObject<UDMBrainSubsystem>();

    UGCCharacterSheet* Player = NewObject<UGCCharacterSheet>();
    Player->CurrentHP = 20;
    Player->MaxHP = 20;
    Brain->SetRuleContext(Player);

    FDMResponse Parsed;
    Parsed.bValid = true;
    Parsed.Narration = TEXT("You shoulder the sealed gate.");
    Parsed.Check.bCheckRequired = true;
    Parsed.Check.CheckType = TEXT("athletics");
    Parsed.Check.DC = 100;
    Parsed.FailureBranch.Narration = TEXT("The gate slams your shoulder.");

    FDMWorldChange Damage;
    Damage.Type = TEXT("damage");
    Damage.Key = TEXT("player");
    Damage.Value = TEXT("4");
    Parsed.WorldChanges.Add(Damage);

    Brain->ResolveParsedResponse(Parsed);

    TestEqual(TEXT("Failure path should apply damage world change"), Player->CurrentHP, 16);
    return true;
}

#endif
