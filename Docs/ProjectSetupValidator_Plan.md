# Project Setup Validator — Plan

Lightweight validation that the project scaffolding is intact. Designed to catch
misconfigured maps, missing actors, and broken engine defaults early (especially
after merges or fresh clones).

## Checks

| # | Check | Method | Severity |
|---|-------|--------|----------|
| 1 | `/Game/Maps/L_Persistent` exists | Filesystem check (`IFileManager`) | Warning* |
| 2 | `PlayerStart` actor present in L_Persistent | World iteration / `TActorIterator` | Fatal |
| 3 | `GCTavernLevelBootstrap` actor present | World iteration / `TActorIterator` | Fatal |
| 4 | `GameDefaultMap` set to L_Persistent | `UGameMapsSettings` | Warning |
| 5 | `GlobalDefaultGameMode` set to GCGameMode | `UGameMapsSettings` | Warning |
| 6 | `GameInstanceClass` set to GCGameInstance | `UGameMapsSettings` | Warning |

## Implementation Status

- **Checks 4-6 (config validation)**: Implemented as `GreymawChronicles.Setup.ConfigDefaults`
  automation test. Runs in headless NullRHI CI.
- **Check 1 (map exists)**: Implemented as `GreymawChronicles.Setup.MapPackageExists`.
  Uses direct filesystem check via `IFileManager::Get().FileExists()` because the asset
  registry is not populated in NullRHI headless mode. Currently emits a **warning** (not
  a failure) because the `L_Persistent.umap` asset has not been committed to the repo yet.
  Change to `TestTrue` once the map file is present.
- **Checks 2-3 (actor validation)**: Require a loaded world with actors. These cannot
  run reliably in NullRHI headless mode without a full map load. Planned as an
  Editor Utility Widget (Blutility) or PIE-mode validation step.

## Future Extensions

- Validate all DataTables referenced by CombatResolver exist and have expected columns
- Check that all NPC blueprint classes referenced in level scripting are compilable
- Verify spell/ability data assets parse correctly
- Add a `/Script/RemoteControl` endpoint that returns project health as JSON
