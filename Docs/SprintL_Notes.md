# Sprint L — Combat, Quest Journal, Scene Transition, Save UI, Audio, Dialogue, Polish

**Date:** 2026-02-22
**Branch:** master

---

## Summary

Sprint L transforms the project from a single-room vertical slice into a multi-scene game with turn-based combat, a quest journal, branching dialogue, multi-slot saves, and an audio framework. The goblin ambush on the Greymaw Trail is the first real combat encounter, using the existing `UCombatResolver`/`UDiceRoller`/`USpellSystem` stack. 15 new tests bring the total to 61.

---

## New Files (15)

| File | Purpose |
|------|---------|
| `Public/Combat/GCEnemyTemplate.h` | Enemy stat block struct with `MakeGoblin()` helper |
| `Public/Combat/GCCombatEncounter.h` | Turn-based combat encounter manager |
| `Private/Combat/GCCombatEncounter.cpp` | Initiative, turn cycling, victory/defeat logic |
| `Public/Quest/GCQuestSubsystem.h` | Quest tracking subsystem (CRUD + JSON serialization) |
| `Private/Quest/GCQuestSubsystem.cpp` | Quest lifecycle and persistence |
| `Public/UI/GCQuestJournalWidget.h` | Quest journal widget (VerticalBox listing) |
| `Private/UI/GCQuestJournalWidget.cpp` | Quest journal UI with toggle visibility |
| `Public/UI/GCSaveLoadWidget.h` | Multi-slot save/load widget (3 slots) |
| `Private/UI/GCSaveLoadWidget.cpp` | Save/load UI with slot summaries |
| `Public/Audio/GCAudioSubsystem.h` | Audio event enum + subsystem (16 events) |
| `Private/Audio/GCAudioSubsystem.cpp` | Null-safe audio playback via PlaySound2D |
| `Public/DungeonMaster/DMDialogueTree.h` | Branching dialogue structs + tree manager |
| `Private/DungeonMaster/DMDialogueTree.cpp` | State-aware option filtering, Marta/Durgan content |
| `Public/Scene/GCScenePropData.h` | Scene prop data table row (FTableRowBase) |
| `Private/Tests/GCSprintLTests.cpp` | 15 new tests |

## Modified Files (12)

| File | Changes |
|------|---------|
| `Public/DungeonMaster/DMBrainSubsystem.h` | +ActiveEncounter, +DialogueTree, +OnCombatStateChanged delegate, +TryHandleCombatInput |
| `Private/DungeonMaster/DMBrainSubsystem.cpp` | Combat routing, quest handlers, scene transition, dialogue tree integration, ~200 new lines |
| `Public/DungeonMaster/DMNarrationPool.h` | +PopulateTrailDefaults, +PopulateCombatDefaults, +PopulateQuestDefaults |
| `Private/DungeonMaster/DMNarrationPool.cpp` | +combat slots (7 slots), +trail slots (4 slots), +quest slots (2 slots), ~45 new variants |
| `Public/Core/GreymawSaveGame.h` | +QuestLogJSON, +SlotDisplayName, +ActiveSceneId, +MakeSlotName(int32) |
| `Private/Core/GreymawSaveGame.cpp` | MakeSlotName implementation with clamping |
| `Public/Core/GCGameInstance.h` | +SaveToSlot, +LoadFromSlot, +HasSaveInSlot |
| `Private/Core/GCGameInstance.cpp` | Multi-slot save/load, quest serialization in save/load |
| `Public/UI/GCMainHUDWidget.h` | +QuestJournal, +SaveLoadPanel BindWidgetOptional slots |
| `Private/UI/GCMainHUDWidget.cpp` | Wire QuestJournal and SaveLoadPanel fallback widgets |
| `Private/UI/GCActionFeedbackWidget.cpp` | Combat state toast (red start, green end), bind OnCombatStateChanged |
| `Private/Scene/SceneDirectorSubsystem.cpp` | WorldState sync on scene load (scene.current) |

---

## Phase 1: Combat Encounter System

First combat encounter: turn-based, using existing `UCombatResolver`/`UDiceRoller`.

**FGCEnemyTemplate** — Lightweight enemy stat block:
- CreatureName, MaxHP, CurrentHP, ArmorClass, AttackModifier, FWeaponAttackData Weapon, InitiativeModifier
- `MakeGoblin()` static helper: HP 7, AC 15, Scimitar 1d6+2, Initiative +2

**UGCCombatEncounter** — Turn-based combat manager:
- `InitializeEncounter(Player, Enemies, DiceRoller)` — stores roster and dice
- `RollInitiative()` — d20+modifier for each combatant, sorts descending
- `AdvanceTurn()` — cycles turn index, increments round on wrap
- `GetOutcome()` — Victory (all enemies HP <= 0), Defeat (player HP <= 0), Ongoing

**Combat routing in DMBrainSubsystem:**
- When `ActiveEncounter` is active and not over, all player input routes to `TryHandleCombatInput()`
- "attack" — d20 + player STR modifier vs enemy AC, damage with DiceRoller
- "flee" — exits combat, returns to exploration
- Enemy turns auto-resolve after player action
- Victory: loot narration, clear combat state, auto-save
- Defeat: soft reset (player HP = 1), return to tavern

**Scripted encounter: "Goblin Ambush"**
- Triggered on trail entry or `challenge goblin` intent
- 2 goblins: HP 7, AC 15, Scimitar 1d6+2
- Victory -> loot + XP narration; Defeat -> "you wake up back in the tavern"

---

## Phase 2: Quest Journal Subsystem

**UGCQuestSubsystem** (UGameInstanceSubsystem):
- `StartQuest()`, `UpdateObjective()`, `CompleteQuest()`, `FailQuest()`
- `GetQuestStatus()`, `GetActiveQuests()`, `GetAllQuests()`, `FindQuest()`
- `ToJSON()` / `FromJSON()` for save/load integration
- `OnQuestUpdated` delegate for UI updates

**UGCQuestJournalWidget:**
- VerticalBox listing quests (Title, Objective, Status)
- Toggle visibility via method call
- Hidden by default (Collapsed)

**Integration:**
- Accept handler calls `StartQuest("tavern_investigation", ...)`
- Report handler calls `CompleteQuest("tavern_investigation")`
- Quest data serialized in save game via `QuestLogJSON`

---

## Phase 3: Scene Transition — Greymaw Trail

- "leave tavern" / "go outside" / "head to trail" triggers scene transition
- Sets `WorldState("scene", "current", "greymaw_trail")`
- Auto-triggers goblin ambush combat
- Blocked during active combat
- Trail narration pool: `trail_arrive`, `trail_look`, `trail_cave_entrance`, `trail_ambient` (12 variants)

---

## Phase 4: Multi-Slot Save System

- 3 manual slots: `GreymawSlot_0`, `GreymawSlot_1`, `GreymawSlot_2`
- Existing auto-save continues as `GreymawSaveSlot_0`
- `MakeSlotName(int32)` with clamping (0-2)
- `SaveToSlot()`, `LoadFromSlot()`, `HasSaveInSlot()` on GCGameInstance
- Quest data persists via `QuestLogJSON` field on save game
- **UGCSaveLoadWidget** — 3 slot entries with Save/Load buttons, hidden by default

---

## Phase 5: Audio/SFX Framework

**EGCAudioEvent** — 16 audio event types:
- UI: ButtonClick, SaveConfirm, QuestUpdate
- Combat: DiceRoll, Hit, Miss, CriticalHit, Victory, Defeat
- Ambience: Tavern, Trail
- Narration: TypewriterTick, NewScene
- Music: TavernTheme, CombatTheme, ExplorationTheme

**UGCAudioSubsystem:**
- `PlayEvent()`, `StopEvent()`, `RegisterSoundForEvent()`, `SetMasterVolume()`
- Null-safe: unregistered or null sounds log warning, no crash
- Under NullRHI: `PlaySound2D` is a no-op, tests pass headless
- Actual .wav/.ogg assets are manual import later

---

## Phase 6: Dialogue Tree System

**UDMDialogueTree** — State-aware branching dialogue:
- `FGCDialogueNode` — NodeId, NPCGreeting, array of options
- `FGCDialogueOption` — OptionText, RequiredState (Category/Key/Value), ResponseNarration, WorldChanges, NextNodeId
- `GetAvailableOptions(NodeId, WorldState)` — filters options by state requirements
- Null WorldState = only unconditional options shown

**Marta dialogue tree (4 options):**
1. "Tell me about the missing villagers" — reveals lore
2. "What's good to eat?" — order food flow
3. "I've been investigating" — requires `task:tavern_investigation=accepted`
4. "Never mind" — end conversation

**Durgan dialogue tree (3 options):**
1. "What do you know about the Greymaw?" — lore
2. "Any advice for a traveler?" — tips
3. "Farewell" — end conversation

---

## Phase 7: Asset Prep + Scene Prop Data

**FGCScenePropRow** (FTableRowBase):
- PropId, DisplayName, MeshAssetPath (empty until meshes imported), DefaultLocation, DefaultRotation, bInteractable
- Prepares for Stylized Medieval Tavern asset pack import

---

## Phase 8: Polish + Integration

- Combat exit guard: victory/defeat clears WorldState, switches music context, auto-saves
- Empty input during combat: prompts "What do you do? (attack, cast, flee)"
- Scene transition blocked during active combat
- Combat toasts: red for damage taken / combat start, green for damage dealt / combat end (4s duration)
- QuestJournal and SaveLoadPanel wired as BindWidgetOptional on MainHUD

---

## Tests

**15 new tests** (61 total: 21 Sprint H + 10 Sprint I + 10 Sprint J + 12 Sprint K - 7 superseded + 15 Sprint L = 61)

| # | Test | Validates |
|---|------|-----------|
| 1 | Combat.EnemyTemplateDefaults | MakeGoblin() stats correct |
| 2 | Combat.InitiativeOrder | RollInitiative produces sorted 3-entry TurnOrder |
| 3 | Combat.TurnAdvancement | AdvanceTurn cycles, wraps, increments round |
| 4 | Combat.VictoryCondition | All enemies HP <= 0 -> Victory |
| 5 | Combat.DefeatCondition | Player HP <= 0 -> Defeat |
| 6 | Quest.StartAndQuery | StartQuest -> Active status, in active list |
| 7 | Quest.UpdateObjective | UpdateObjective changes CurrentObjective |
| 8 | Quest.CompleteQuest | CompleteQuest -> Completed, removed from active |
| 9 | Scene.TrailNarrationExists | PopulateTrailDefaults, 4 slots return non-empty |
| 10 | Scene.SceneIdTracking | WorldState scene.current round-trip |
| 11 | Save.MultiSlotNaming | MakeSlotName(0/1/2) + clamping |
| 12 | Save.QuestLogPersistence | Quest ToJSON/FromJSON round-trip |
| 13 | Audio.NullSafePlayback | PlayEvent with no registered sound -> no crash |
| 14 | Dialogue.OptionFiltering | 3 options without state, 4 with state |
| 15 | Dialogue.NodeChaining | Option NextNodeId chains to correct node |

---

## Known Limitations

- Combat uses manual d20 rolls via DiceRoller, not CombatResolver (enemies lack CharacterSheets).
- Only one combat encounter scripted (goblin ambush). Framework supports more.
- Dialogue trees are code-defined (Marta, Durgan). Data-driven loading from assets is future work.
- Audio subsystem is code framework only — no .wav/.ogg assets included.
- Scene transition is narrative-only (no level streaming or new geometry).
- Quest journal UI is code-constructed fallback (no UMG Blueprint).
- Save slot UI is code-constructed fallback (no UMG Blueprint).
- Scene prop data has empty MeshAssetPath — meshes not yet imported.
