#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGreymawMilestone5SmokeTest, "Greymaw.M5.Smoke", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FGreymawMilestone5SmokeTest::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("Milestone 5 smoke test placeholder"), true);
    return true;
}

#endif
