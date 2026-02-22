#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "DungeonMaster/DMIntentClassifier.h"

/**
 * Sprint H: Integration tests for the intent classification and scripted loop.
 * Run via: UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests Greymaw.Intent"
 */

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIntentLookTest, "Greymaw.Intent.LookAround",
    EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)

bool FIntentLookTest::RunTest(const FString& Parameters)
{
    UDMIntentClassifier* Classifier = NewObject<UDMIntentClassifier>();

    // Exact match
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("look around the room"));
        TestEqual(TEXT("look around -> Look intent"), static_cast<uint8>(R.Intent), static_cast<uint8>(EDMIntent::Look));
        TestTrue(TEXT("confidence >= 0.5"), R.Confidence >= 0.5f);
    }

    // Fuzzy: "survey" should also map to Look
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("survey the tavern"));
        TestEqual(TEXT("survey -> Look intent"), static_cast<uint8>(R.Intent), static_cast<uint8>(EDMIntent::Look));
    }

    // "where am i" variant
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("where am i"));
        TestEqual(TEXT("where am i -> Look intent"), static_cast<uint8>(R.Intent), static_cast<uint8>(EDMIntent::Look));
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIntentTalkTest, "Greymaw.Intent.TalkToNPC",
    EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)

bool FIntentTalkTest::RunTest(const FString& Parameters)
{
    UDMIntentClassifier* Classifier = NewObject<UDMIntentClassifier>();

    // "talk to marta" -> Talk with subject marta
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("talk to marta"));
        TestEqual(TEXT("talk to marta -> Talk"), static_cast<uint8>(R.Intent), static_cast<uint8>(EDMIntent::Talk));
        TestTrue(TEXT("subject contains marta"), R.Subject.Contains(TEXT("marta")));
    }

    // "ask the old man about the greymaw"
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("ask the old man about the greymaw"));
        TestEqual(TEXT("ask old man -> Talk"), static_cast<uint8>(R.Intent), static_cast<uint8>(EDMIntent::Talk));
        TestTrue(TEXT("subject contains old man"), R.Subject.Contains(TEXT("old man")));
    }

    // "chat with kael"
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("chat with kael"));
        TestEqual(TEXT("chat with kael -> Talk"), static_cast<uint8>(R.Intent), static_cast<uint8>(EDMIntent::Talk));
        TestTrue(TEXT("subject contains kael"), R.Subject.Contains(TEXT("kael")));
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIntentChallengeTest, "Greymaw.Intent.ChallengeAction",
    EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)

bool FIntentChallengeTest::RunTest(const FString& Parameters)
{
    UDMIntentClassifier* Classifier = NewObject<UDMIntentClassifier>();

    // "arm wrestle" -> Challenge
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("arm wrestle kael"));
        TestEqual(TEXT("arm wrestle -> Challenge"), static_cast<uint8>(R.Intent), static_cast<uint8>(EDMIntent::Challenge));
    }

    // "intimidate the drunk" -> Challenge
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("intimidate the drunk"));
        TestEqual(TEXT("intimidate -> Challenge"), static_cast<uint8>(R.Intent), static_cast<uint8>(EDMIntent::Challenge));
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIntentFallbackTest, "Greymaw.Intent.FallbackPath",
    EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)

bool FIntentFallbackTest::RunTest(const FString& Parameters)
{
    UDMIntentClassifier* Classifier = NewObject<UDMIntentClassifier>();

    // Gibberish should fall back to Unknown
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("xyzzy plugh"));
        TestEqual(TEXT("gibberish -> Unknown"), static_cast<uint8>(R.Intent), static_cast<uint8>(EDMIntent::Unknown));
    }

    // Empty string
    {
        const FDMIntentResult R = Classifier->Classify(TEXT(""));
        TestEqual(TEXT("empty -> Unknown"), static_cast<uint8>(R.Intent), static_cast<uint8>(EDMIntent::Unknown));
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIntentMoveTest, "Greymaw.Intent.MoveAction",
    EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)

bool FIntentMoveTest::RunTest(const FString& Parameters)
{
    UDMIntentClassifier* Classifier = NewObject<UDMIntentClassifier>();

    // "go to the bar"
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("go to the bar"));
        TestEqual(TEXT("go to bar -> Move"), static_cast<uint8>(R.Intent), static_cast<uint8>(EDMIntent::Move));
        TestTrue(TEXT("subject contains bar"), R.Subject.Contains(TEXT("bar")));
    }

    // "approach the door"
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("approach the door"));
        TestEqual(TEXT("approach door -> Move"), static_cast<uint8>(R.Intent), static_cast<uint8>(EDMIntent::Move));
        TestTrue(TEXT("subject contains door"), R.Subject.Contains(TEXT("door")));
    }

    // "sit down"
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("sit down at a table"));
        TestEqual(TEXT("sit -> Move"), static_cast<uint8>(R.Intent), static_cast<uint8>(EDMIntent::Move));
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FIntentInspectCheckTest, "Greymaw.Intent.InspectTriggersCheck",
    EAutomationTestFlags::ProductFilter | EAutomationTestFlags::EditorContext)

bool FIntentInspectCheckTest::RunTest(const FString& Parameters)
{
    UDMIntentClassifier* Classifier = NewObject<UDMIntentClassifier>();

    // "examine the ale" -> Inspect with subject ale (this triggers a perception check in-game)
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("examine the ale"));
        TestEqual(TEXT("examine ale -> Inspect"), static_cast<uint8>(R.Intent), static_cast<uint8>(EDMIntent::Inspect));
        TestTrue(TEXT("subject contains ale"), R.Subject.Contains(TEXT("ale")));
    }

    // "read the quest board" -> Inspect
    {
        const FDMIntentResult R = Classifier->Classify(TEXT("read the quest board"));
        TestEqual(TEXT("read quest board -> Inspect"), static_cast<uint8>(R.Intent), static_cast<uint8>(EDMIntent::Inspect));
        TestTrue(TEXT("subject contains quest board"), R.Subject.Contains(TEXT("quest board")));
    }

    return true;
}
