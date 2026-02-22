#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "GameMapsSettings.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"

/**
 * Project Setup Validator — Config Defaults
 *
 * Validates that critical engine config values (Maps & Modes, GameInstance)
 * are set correctly. These checks run in headless NullRHI CI and catch
 * accidental config regressions after merges or fresh clones.
 *
 * See: Docs/ProjectSetupValidator_Plan.md
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSetupConfigDefaultsTest,
    "GreymawChronicles.Setup.ConfigDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSetupConfigDefaultsTest::RunTest(const FString& Parameters)
{
    const UGameMapsSettings* MapsSettings = GetDefault<UGameMapsSettings>();

    // Check GameDefaultMap points to L_Persistent (static accessor — GameDefaultMap is private)
    const FString DefaultMap = UGameMapsSettings::GetGameDefaultMap();
    TestTrue(
        TEXT("GameDefaultMap should reference L_Persistent"),
        DefaultMap.Contains(TEXT("L_Persistent"))
    );

    // Check GlobalDefaultGameMode is GCGameMode (static accessor — field is private)
    const FString GameModeClass = UGameMapsSettings::GetGlobalDefaultGameMode();
    TestTrue(
        TEXT("GlobalDefaultGameMode should be GCGameMode"),
        GameModeClass.Contains(TEXT("GCGameMode"))
    );

    // Check GameInstanceClass is GCGameInstance (public field)
    const FSoftClassPath GameInstanceClass = MapsSettings->GameInstanceClass;
    TestTrue(
        TEXT("GameInstanceClass should be GCGameInstance"),
        GameInstanceClass.ToString().Contains(TEXT("GCGameInstance"))
    );

#if WITH_EDITORONLY_DATA
    // EditorStartupMap is only available in editor builds
    const FSoftObjectPath EditorMap = MapsSettings->EditorStartupMap;
    TestTrue(
        TEXT("EditorStartupMap should reference L_Persistent"),
        EditorMap.ToString().Contains(TEXT("L_Persistent"))
    );
#endif

    return true;
}

/**
 * Project Setup Validator — Map Asset Exists
 *
 * Verifies that the L_Persistent map asset file (.umap) exists on disk.
 * Uses direct filesystem check instead of FPackageName::DoesPackageExist
 * because the asset registry is not populated in NullRHI headless mode.
 *
 * Enforces that the map file is present — fails CI if missing.
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FSetupMapExistsTest,
    "GreymawChronicles.Setup.MapPackageExists",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FSetupMapExistsTest::RunTest(const FString& Parameters)
{
    // Resolve to absolute path — FPaths::ProjectContentDir() may be relative
    const FString MapAssetPath = FPaths::ConvertRelativePathToFull(
        FPaths::Combine(
            FPaths::ProjectContentDir(), TEXT("Maps"), TEXT("L_Persistent.umap")
        )
    );

    const bool bExists = IFileManager::Get().FileExists(*MapAssetPath);
    TestTrue(
        FString::Printf(TEXT("Map file should exist at: %s"), *MapAssetPath),
        bExists
    );

    return true;
}

#endif
