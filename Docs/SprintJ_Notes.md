# Sprint J — NPC Spawning, Inventory Tracking, Save/Load, Asset Prep

**Date:** 2026-02-22
**Branch:** master

---

## Summary

Sprint J resolves the five remaining first-playable blockers: NPC actors now spawn as tagged skeletal-mesh actors in the tavern, inventory items are tracked and surfaced via toast, world state serializes to/from JSON for persistence, a full save/load system auto-saves after each turn and auto-loads on startup, and the tavern bootstrap is prepared for the Stylized Medieval Tavern asset pack.

---

## New Files

| File | Purpose |
|------|---------|
| `Public/Core/GreymawSaveGame.h` | USaveGame subclass (CharacterSheet, WorldState, ConversationHistory, Timestamp) |
| `Private/Core/GreymawSaveGame.cpp` | SaveSlotName constant ("GreymawAutoSave") |
| `Private/Tests/GCSprintJTests.cpp` | 10 new tests covering all Sprint J features |
| `Docs/SprintJ_Notes.md` | This file |

## Modified Files

| File | Changes |
|------|---------|
| `Public/DungeonMaster/DMWorldStateSubsystem.h` | +ToJSON, +FromJSON, +ClearAll |
| `Private/DungeonMaster/DMWorldStateSubsystem.cpp` | Implement JSON serialization/deserialization |
| `Public/DungeonMaster/DMBrainSubsystem.h` | +OnInventoryChanged delegate, +GetPlayerSheet, +GetConversationHistory |
| `Private/DungeonMaster/DMBrainSubsystem.cpp` | +inventory_add handler in ApplyWorldChanges + broadcast |
| `Public/Core/GCGameInstance.h` | +SaveGame, +LoadGame, +HasSaveGame, +HandleActionSequenceComplete |
| `Private/Core/GCGameInstance.cpp` | Full save/load implementation with auto-save/auto-load hooks |
| `Public/Scene/GCTavernLevelBootstrap.h` | +SpawnNPCs, +SpawnNPCActor, +NPC location properties |
| `Private/Scene/GCTavernLevelBootstrap.cpp` | NPC spawn with mannequin mesh + tint + tags |
| `Public/UI/GCActionFeedbackWidget.h` | +HandleInventoryChanged |
| `Private/UI/GCActionFeedbackWidget.cpp` | Bind OnInventoryChanged delegate, push "Acquired" toast |
| `Docs/FirstPlayable_QuickTest.md` | +prompts 22-26, updated counts and known limitations |

---

## Phase 1: WorldState Serialization

`UDMWorldStateSubsystem` now supports full round-trip serialization.

- **`ToJSON()`** — Iterates `StateStore`, builds nested FJsonObject (category -> key/value), serializes via `FJsonSerializer`.
- **`FromJSON(JsonText)`** — Calls `ClearAll()`, deserializes JSON, repopulates `StateStore`. Returns false on parse failure. Empty string is valid (no state).
- **`ClearAll()`** — `StateStore.Empty()`.

JSON format:
```json
{
  "npc_disposition": { "marta": "friendly", "durgan": "open" },
  "world_flags": { "coin_found": "true" }
}
```

---

## Phase 2: Inventory Tracking

`ApplyWorldChanges()` in DMBrainSubsystem now handles `inventory_add` world changes:

- When `Type == "inventory_add"` and `Key == "player"`, calls `PlayerSheet->Equipment.AddUnique(Value)`.
- Broadcasts `OnInventoryChanged` delegate with the item name.
- `GCActionFeedbackWidget` binds this delegate and pushes a green "Acquired: stolen coin pouch" toast.

---

## Phase 3: Save/Load System

**UGreymawSaveGame** — `USaveGame` subclass with 4 fields:
- `CharacterSheetJSON` — serialized via `UGCCharacterSheet::ToCompactJSON()`
- `WorldStateJSON` — serialized via `UDMWorldStateSubsystem::ToJSON()`
- `ConversationHistoryJSON` — serialized via `UDMConversationHistory::SerializeToJSON()`
- `SaveTimestamp` — ISO 8601 UTC string

**Auto-save:** `GCGameInstance` binds `ActionDirector->OnActionSequenceComplete` -> `SaveGame()`.
**Auto-load:** `GCGameInstance::Init()` calls `LoadGame()` if a save slot exists.

---

## Phase 4: NPC Spawning

Three NPCs are spawned in `GCTavernLevelBootstrap::SpawnNPCs()`:

| NPC | Location | Rotation | Tint | Role |
|-----|----------|----------|------|------|
| Marta | (320, 30, 0) | 180deg | Warm red | Barkeep |
| Durgan | (0, -200, 0) | 45deg | Blue-grey | Storyteller |
| Kael | (-150, 180, 0) | -90deg | Dark green | Mercenary |

Implementation:
- `AActor` with `USkeletalMeshComponent` (not ACharacter — no navmesh needed for stationary NPCs)
- Loads `SKM_Manny` from `/Game/` path, falls back to `/Engine/` path
- Creates `UMaterialInstanceDynamic` for color tint (tries BodyColor, Base Color, Tint param names)
- Adds NPC name as actor tag for `ResolveActor()` lookup
- If mesh not found (headless builds), NPC exists as invisible tagged actor — tests still work

---

## Phase 5: Tests

**10 new tests** (41 total: 21 Sprint H + 10 Sprint I + 10 Sprint J)

| # | Test | Validates |
|---|------|-----------|
| 1 | WorldState.Serialization | ToJSON/FromJSON round-trip with 3 entries |
| 2 | WorldState.ClearAll | ClearAll empties all categories |
| 3 | Inventory.AddItem | Equipment.AddUnique prevents duplicates |
| 4 | Inventory.JSONRoundTrip | Equipment survives CharacterSheet serialization |
| 5 | SaveLoad.ObjectCreation | UGreymawSaveGame fields + slot name constant |
| 6 | SaveLoad.ConversationRoundTrip | ConversationHistory serialize/deserialize |
| 7 | NPCSpawn.TagsRegistered | "talk to marta" extracts subject correctly |
| 8 | NPCSpawn.NameConsistency | All 3 NPC names resolve via intent classifier |
| 9 | Inventory.DelegateExists | OnInventoryChanged delegate is broadcastable |
| 10 | WorldState.InvalidJSON | FromJSON handles malformed input gracefully |

---

## Phase 6: Tavern Asset Prep

- `GCTavernLevelBootstrap` is ready for the Stylized Medieval Tavern asset pack (Baturinets, Fab).
- Asset lists UE 4.27-5.4 but content assets are forward-compatible with UE 5.7.
- **Workaround:** Create a dummy UE 5.4 project, add asset there, copy Content folder.
- `Content/Environment/StylizedMedievalTavern/.gitkeep` created as placeholder.
- Existing cube geometry used as fallback until tavern meshes are imported.

---

## Blockers Resolved

| # | Blocker | Resolution |
|---|---------|------------|
| 1 | NPC actors don't exist | SpawnNPCs() in TavernLevelBootstrap |
| 2 | Inventory not tracked | inventory_add handler + Equipment.AddUnique |
| 3 | No save/load | GreymawSaveGame + auto-save/load in GCGameInstance |
| 4 | No skeletal meshes | SKM_Manny with dynamic material tint |
| 5 | Camera targets phantom NPCs | Fixed by blocker 1 — ResolveActor finds tagged actors |

---

## Known Limitations

- Mannequin mesh may not exist in all project configurations (NPC becomes invisible tagged actor).
- Dynamic material tint is best-effort — depends on material parameter names.
- Save slot is single-slot auto-save only (no manual save management UI).
- Tavern asset pack requires manual import step (UE 5.4 compatibility workaround).
