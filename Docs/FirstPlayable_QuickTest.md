# First Playable - Quick Test Script

**Sprint:** L
**Time:** ~8 minutes
**Prerequisite:** Successful Development Editor build (`GreymawChroniclesEditor Win64 Development`)

---

## Setup

1. Open the project in UE 5.7 Editor.
2. The default map should be `L_Persistent` (set in `DefaultEngine.ini`). If it is empty, that is expected.
3. Place a `GCTavernLevelBootstrap` actor anywhere in the level (search Place Actors panel).
4. Press **Play (PIE)**.

## What You Should See

| Element | Location | Description |
|---------|----------|-------------|
| Floor plane | Origin | White cube scaled 20x20x1 |
| Directional + Sky light | Above | Basic scene lighting |
| Bar marker | ~(350, 0, 50) | Cube tagged `bar` |
| Player pawn | ~(0, 0, 100) | Default pawn at PlayerStart |
| HUD Overlay | Top-left (20, 20) | HP bar (green, 20/20) |
| Debug Overlay | Top-right (800, 20) | `[DM] Idle`, `[Actions] 0 queued`, `[Check] ---` |
| Action Feedback | Center-right (860, 300) | Transient toast messages (auto-fade) |
| Narration Panel | Bottom (40, 620) | Typewriter text area |
| Input Box | Bottom (40, 790) | Player text input |

## Prompt Sequence

Type each prompt into the input box and press Enter. Verify the expected response.
**Note:** Sprint H uses fuzzy intent matching. Exact phrasing is not required — synonyms and natural variations work.

### 1. `look around` (or `survey the room`, `where am i`, `observe`)
- **Narration:** Describes the Thornhaven taproom (varies each time — 3 variants).
- **Debug:** `[DM] Processing...` flashes, then `[DM] Idle`. `[Actions] 1 queued`.
- **Toast:** `> DM is resolving...` then `> Ready for input`.

### 2. `talk to marta` (or `speak to marta`, `ask marta`, `chat with marta`)
- **Narration:** Marta describes missing villagers, offers 50 gold quest (varies — 3 variants).
- **Debug:** `[Actions] 3 queued` (move + talk_gesture + nod).
- **Action:** Player lerps toward bar position (320, 0, 100).
- **Toast:** `> Moving player to target`.

### 3. `go to the bar` (or `walk to bar`, `approach bar`, `head to bar`)
- **Narration:** Floorboards creak, regulars size you up (varies — 3 variants).
- **Debug:** `[Actions] 3 queued` (move + wait + idle).
- **Action:** Player lerps to (350, 0, 100).

### 4. `arm wrestle kael` (or `challenge kael`, `wrestle`, `spar with kael`)
- **Narration:** Kael setup text (varies). Then success or failure branch.
- **Debug:** `[Check] athletics | Roll:X + Mod:3 = Y vs DC 14  PASS/FAIL`.
- **Toast:** `> athletics check: Y vs DC 14 -- PASSED/FAILED`.
- **Color:** Green on PASS, red on FAIL. `CRITICAL!` or `FUMBLE!` on nat 20/1.

### 5. `examine the ale` (or `inspect the mug`, `study the drink`) — NEW micro-event
- **Narration:** "Something catches your eye — a faint shimmer beneath the foam."
- **Debug:** `[Check] perception | Roll:X + Mod:Y = Z vs DC 12  PASS/FAIL`.
- **Branch (PASS):** Spot a rune in the mug. Marta nods.
- **Branch (FAIL):** Shimmer fades. Normal ale.
- **Toast:** `> perception check: Z vs DC 12 -- PASSED/FAILED`.

### 6. `intimidate` (or `threaten`, `challenge the drunk`) — NEW micro-event
- **Narration:** A rowdy patron causing trouble. You step forward.
- **Debug:** `[Check] intimidation | Roll:X + Mod:Y = Z vs DC 13  PASS/FAIL`.
- **Branch (PASS):** Patron backs down. Marta nods gratefully.
- **Branch (FAIL):** "Mind your business, outsider."
- **Toast:** `> intimidation check: Z vs DC 13 -- PASSED/FAILED`.

### 7. `grab it` (or `pick up`, `use`, `take`) — NEW micro-event
- **Narration:** A coin glints under a table. A shadowed hand reaches for it too.
- **Debug:** `[Check] sleight_of_hand | Roll:X + Mod:Y = Z vs DC 11  PASS/FAIL`.
- **Branch (PASS):** You grab the mysterious coin first.
- **Branch (FAIL):** The shadow snatches it away.

### 8. `talk to kael` (or `speak to kael`, `chat with kael`)
- **Narration:** Kael offers to join your expedition (varies — 3 variants).
- **Debug:** `[Actions] 1 queued`.

### 9. `ask the old man` (or `talk to durgan`, `speak to durgan`)
- **Narration:** Durgan describes the Greymaw (varies — 3 variants).
- **Debug:** `[Actions] 1 queued`.

### 10. `read the quest board` (or `examine notice board`, `inspect board`)
- **Narration:** Missing villagers notice (varies — 3 variants).
- **Debug:** `[Actions] 1 queued`.

### 11. `help durgan` (or `assist the old man`, `comfort durgan`)
- **Narration:** You steady Durgan's hand (varies — 3 variants). Pure narrative, no check.

### 12. `drink the ale` (or `use the mug`, `have a drink`)
- **Narration:** Marta slides a foaming mug your way (varies — 3 variants). Pure narrative.

### 13. `go to the door` (or `approach the door`, `head to the door`)
- **Narration:** Rain hammers outside. The Greymaw trail beckons. Player moves to door position.

### 14. `look at the hearth` (or `examine the fire`, `inspect the fireplace`)
- **Narration:** Fire description with carved initials / faded painting (varies — 3 variants).

### 15. `xyzzy` (gibberish / catch-all)
- **Narration:** Fallback reminder of available NPCs and objects (varies — 3 variants).

### 16. `order some food` (or `buy stew`, `request bread`, `purchase a meal`) — Sprint I
- **Narration:** Marta serves food (varies — 3 variants from `order_food` pool).
- **State:** Sets `npc_disposition:marta=friendly`. Subsequent `talk to marta` uses warm narration.
- **If Marta is suspicious:** Order is refused (3 variants from `order_refused` pool).

### 17. `pickpocket marta` (or `steal from marta`, `pilfer her pouch`) — Sprint I
- **Narration:** Steal setup text (varies — 3 variants).
- **Check:** Sleight of Hand DC 14 (DC varies: Kael=16, Durgan=10, generic=13).
- **Branch (PASS):** Stolen coin pouch. **Branch (FAIL):** Target becomes suspicious.
- **State on FAIL:** `npc_disposition:<target>=suspicious`. Marta refuses service if suspicious.

### 18. `eavesdrop` (or `listen in`, `overhear`, `spy on the room`) — Sprint I
- **Narration:** You filter the taproom sounds.
- **Check:** Perception DC 12.
- **Branch (PASS):** Lore snippet (3 variants). **Branch (FAIL):** Only noise (3 variants).

### 19. `convince marta` (or `persuade durgan`, `charm kael`, `bribe`) — Sprint I
- **Narration:** Subject-specific persuasion attempt.
- **Check:** Persuasion — DC varies (Marta=12, Durgan=14, Kael=10).
- **State:** Success on Marta -> `trusting`, success on Durgan -> `open`.
- **Consequence:** `talk to durgan` after successful persuasion uses `talk_durgan_open` (extra lore).

### 20. `rest by the fire` (or `relax`, `meditate`, `sit down`, `take a break`) — Sprint I
- **Narration:** Atmosphere text (varies — 3 variants). No check.
- **Healing:** If HP < MaxHP, restores 1 HP.

### 21. `play dice` (or `gamble`, `bet`, `wager`) — Sprint I
- **Narration:** Dice game setup (varies — 3 variants).
- **Check:** Deception DC 13.
- **Branch (PASS):** Win coins (3 variants). **Branch (FAIL):** Lose coins (3 variants).
- **Special:** `gamble with durgan` — Durgan refuses (pure narrative, no check).

### 22. `steal from durgan` (or `pickpocket durgan`) — Sprint J (Inventory)
- **Check:** Sleight of Hand DC 10.
- **Branch (PASS):** Stolen coin pouch added to inventory. **Green toast:** `Acquired: stolen coin pouch`.
- **Branch (FAIL):** Durgan notices.
- **Equipment:** `UGCCharacterSheet::Equipment` now contains "stolen_coin_pouch".

### 23. `talk to marta` — Sprint J (NPC Spawning)
- **Action:** Camera focuses on Marta actor at (320, 30, 0) — **no fallback warning in log**.
- **Previously:** Camera warned "ResolveActor: falling back to player pawn".
- **Now:** Three colored mannequins visible in the scene.

### 24. `look around` (repeat) — Sprint J (Save/Load)
- **After previous actions**, quit PIE and restart.
- **On restart:** World state, conversation history, and character sheet are **automatically restored**.
- **Verify:** `talk to marta` still uses disposition-aware narration (if set to friendly previously).

### 25. `talk to durgan` — Sprint J (NPC exists)
- **Action:** Camera focuses on Durgan actor at (0, -200, 0).
- **Narration:** Durgan describes the Greymaw (varies — 3 variants).
- **Visual:** Blue-grey tinted mannequin visible.

### 26. `talk to kael` — Sprint J (NPC exists)
- **Action:** Camera focuses on Kael actor at (-150, 180, 0).
- **Narration:** Kael offers expedition help (varies — 3 variants).
- **Visual:** Dark green tinted mannequin visible.

---

## Sprint K Mini-Loop (10-Prompt Focused Flow)

The following 10-prompt sequence exercises all Sprint K features in order. Can be run after or instead of the full 26-prompt sequence above.

### K1. `look around` — Establish scene
- **Narration:** Tavern description (varies). Camera: Establishing mode.

### K2. `talk to marta` — Learn about missing villagers
- **Narration:** Marta mentions missing villagers. **NPC state:** Marta transitions from `neutral` to `engaged`.
- **Verify:** If you `talk to marta` again, narration should use engaged variant (different from first).

### K3. `accept the task` — Accept tavern investigation
- **Intent:** Accept. **Narration:** Task accepted confirmation (3 variants).
- **State:** `task:tavern_investigation = accepted` in WorldState.
- **Toast:** Action feedback confirms acceptance.

### K4. `eavesdrop` — Perception check, clue if pass
- **Check:** Perception DC 12.
- **Branch (PASS):** Overheard snippet + `task_clue:overheard_snippet = true`.
- **Branch (FAIL):** Only tavern noise. No clue set.

### K5. `convince durgan` — Persuasion check, clue if pass
- **Check:** Persuasion DC 14.
- **Branch (PASS):** Durgan shares lore + `task_clue:durgan_lore = true` + Durgan state -> `helpful`.
- **Branch (FAIL):** Durgan refuses. No disposition change.

### K6. `read the quest board` — Clue (no check)
- **Narration:** Notice board details. **State:** `task_clue:board_notice = true` (unconditional).

### K7. `report to marta` — Resolution based on clue count
- **0 clues:** "You haven't learned anything yet." Task remains accepted.
- **1 clue:** Partial resolution narration.
- **2+ clues:** Full resolution narration. `task:tavern_investigation = resolved`.

### K8. `steal from durgan` — Verify bug fix (loot only on pass)
- **Check:** Sleight of Hand DC 10.
- **Branch (PASS):** Stolen coin pouch added to inventory. Green "Acquired" toast. Durgan NOT upset.
- **Branch (FAIL):** NO loot. Durgan becomes suspicious + upset.
- **Bug fix verified:** Loot only on success, suspicious only on failure.

### K9. `talk to durgan` — Verify NPC state routing
- **If K8 passed (no steal fail):** Durgan in `engaged` or `helpful` state -> engaged/friendly narration.
- **If K8 failed:** Durgan in `upset` state -> suspicious narration variant.
- **Verify:** Narration clearly differs based on NPC interaction state.

### K10. Quit + Restart PIE — Verify save/load feedback
- **On save (before quit):** Dim grey "Autosaved" toast (2s, subtle).
- **On restart:** "Loaded save from <timestamp>" toast. World state, conversation, inventory persist.
- **Console:** Type `gc.save` in console -> "Game saved" toast. Type `gc.load` -> "Save loaded" toast.

---

## Sprint L Adventure Flow (8-Prompt Combat + Quest Sequence)

The following sequence exercises all Sprint L features: combat, quest journal, scene transition, and dialogue. Run after or instead of the Sprint K sequence.

### L1. `talk to marta` — Dialogue tree (if engaged/helpful)
- **If NPC state is engaged/helpful:** Numbered dialogue options appear (1-4).
- **Option 1:** "Tell me about the missing villagers" — reveals lore.
- **Option 3:** (Requires `task:tavern_investigation=accepted`) "I've been investigating."
- **Option 4:** "Never mind" — ends conversation.

### L2. `accept the task` — Quest started
- **Intent:** Accept. **Narration:** Task accepted (3 variants).
- **Quest toast:** Quest journal updated.
- **State:** `task:tavern_investigation = accepted`.

### L3. `read the quest board` + `eavesdrop` — Gather clues
- **Board:** `task_clue:board_notice = true`.
- **Eavesdrop:** Perception DC 12. Pass -> `task_clue:overheard_snippet = true`.

### L4. `report to marta` — Quest complete
- **2+ clues:** Full resolution. `task:tavern_investigation = resolved`.
- **Quest status:** Completed in quest journal.

### L5. `leave the tavern` (or `go outside`, `head to the trail`)
- **Guard:** Blocked if combat is active.
- **Narration:** Trail arrival text (3 variants from `trail_arrive` pool).
- **State:** `scene:current = greymaw_trail`.
- **Red toast:** "COMBAT STARTED!" — goblin ambush auto-triggers.
- **Narration:** Combat start text (3 variants from `combat_start` pool).

### L6. `attack` — Player combat turn
- **Roll:** d20 + STR modifier vs Goblin AC 15.
- **Hit:** Damage narration (3 variants). Goblin HP reduced.
- **Miss:** Miss narration (3 variants).
- **Enemy turns:** Auto-resolve after player action. Red toast if player takes damage.

### L7. Repeat `attack` until victory or defeat
- **Victory (all goblins HP <= 0):** Victory narration. Green "COMBAT ENDED" toast. Auto-save.
- **Defeat (player HP <= 0):** Defeat narration. Player HP reset to 1. Soft return to tavern.

### L8. `gc.save 0` / `gc.load 0` — Multi-slot save/load
- **Console:** `gc.save 0` -> saves to GreymawSlot_0. Toast confirms.
- **Console:** `gc.load 0` -> loads from slot. Quest state, combat results persist.
- **Verify:** Quest journal still shows completed quest after load.

---

## Pass Criteria

- All 26 base prompts + 10 Sprint K prompts + 8 Sprint L prompts produce narration text.
- Narration text **varies** on repeated attempts of the same prompt (narration pool).
- Fuzzy matching works: synonyms and natural phrasing resolve to correct intent.
- 3 micro-events trigger ability checks (perception DC 12, intimidation DC 13, sleight_of_hand DC 11).
- 8 Sprint I/K intents trigger correctly: Order, Steal, Listen, Persuade, Rest, Gamble, Accept, Report.
- `arm wrestle` still triggers athletics DC 14 check.
- **NPC state persists within session:** ordering from Marta → friendly narration on next talk; failed steal → suspicious narration + order refusal; successful persuasion of Durgan → extra lore on next talk.
- **Sprint J: NPC actors exist** — 3 colored mannequins visible, camera targets them by tag (no fallback).
- **Sprint J: Inventory tracking** — successful steal adds item to Equipment, green "Acquired" toast appears.
- **Sprint J: Save/Load** — quit and restart PIE, world state and conversation history persist.
- **Sprint K: Bug fix** — steal loot only on success, suspicious only on failure. Persuade disposition only on success.
- **Sprint K: NPC interaction state** — first talk -> engaged, persuade pass -> helpful, steal fail -> upset. Narration routes correctly per state.
- **Sprint K: Task mini-loop** — accept task -> gather clues -> report -> resolution. 2+ clues required for full resolution.
- **Sprint K: Save/Load feedback** — "Autosaved" and "Loaded save from <time>" toasts. `gc.save`/`gc.load` console commands work.
- **Sprint K: Camera polish** — null-safe camera fallback, smoother blend times, no dead-air delays.
- **Sprint L: Combat** — "leave tavern" triggers trail + goblin ambush. "attack"/"flee" work during combat. Victory/defeat resolve correctly.
- **Sprint L: Quest journal** — accept task -> quest started. report -> quest completed. Quest status persists through save/load.
- **Sprint L: Scene transition** — "leave tavern" changes scene to trail. Blocked during combat.
- **Sprint L: Multi-slot save** — `gc.save 0`/`gc.load 0` work. 3 manual slots available.
- **Sprint L: Audio framework** — no crashes from unregistered audio events (null-safe).
- **Sprint L: Dialogue trees** — Marta shows numbered options when engaged/helpful. Conditional options filter by WorldState.
- **Sprint L: Combat toasts** — red "COMBAT STARTED" / green "COMBAT ENDED" toasts appear.
- Action feedback toasts appear mid-right with multi-line check breakdown (3 lines: header/roll/result) and auto-fade after ~5 seconds for checks.
- Debug overlay updates state, action count, and check result correctly.
- Player visually moves during movement-intent prompts.
- Input box disables during processing and re-enables after.
- Input recovers within 20 seconds even if processing state gets stuck.
- No crashes or ensure failures in the Output Log.

## Automated Tests

Run all tests (61 total):
```
UnrealEditor-Cmd.exe <project> -ExecCmds="Automation RunTests GreymawChronicles" -NullRHI -NoSound
```

### Sprint H Tests (21):
- `Greymaw.Intent.LookAround` — look/survey/where am i -> Look intent
- `Greymaw.Intent.TalkToNPC` — talk/ask/chat + subject extraction
- `Greymaw.Intent.ChallengeAction` — arm wrestle/intimidate -> Challenge
- `Greymaw.Intent.FallbackPath` — gibberish/empty -> Unknown
- `Greymaw.Intent.MoveAction` — go/approach/sit -> Move + subject
- `Greymaw.Intent.InspectTriggersCheck` — examine/read + subject extraction
- (+ 15 others: Narration pool, response parser, combat resolver, etc.)

### Sprint I Tests (10):
- `GreymawChronicles.SprintI.Intent.Order` — "order some food" -> Order intent
- `GreymawChronicles.SprintI.Intent.Steal` — "pickpocket marta" -> Steal, subject=marta
- `GreymawChronicles.SprintI.Intent.Listen` — "eavesdrop" -> Listen intent
- `GreymawChronicles.SprintI.Intent.Persuade` — "convince marta" -> Persuade, subject=marta
- `GreymawChronicles.SprintI.Intent.Rest` — "rest by the fire" -> Rest intent
- `GreymawChronicles.SprintI.Intent.Gamble` — "play dice" -> Gamble intent
- `GreymawChronicles.SprintI.WorldState.SetGet` — WorldState set/get/has/clear round-trip
- `GreymawChronicles.SprintI.WorldState.DispositionAffectsNarration` — Disposition-specific narration slots
- `GreymawChronicles.SprintI.NarrationPool.NewSlots` — All 16 new slots return non-empty
- `GreymawChronicles.SprintI.Timeout.ProcessingRecovery` — Existing intents still work (regression)

### Sprint J Tests (10):
- `GreymawChronicles.SprintJ.WorldState.Serialization` — ToJSON/FromJSON round-trip
- `GreymawChronicles.SprintJ.WorldState.ClearAll` — ClearAll empties all state
- `GreymawChronicles.SprintJ.Inventory.AddItem` — Equipment.AddUnique behavior
- `GreymawChronicles.SprintJ.Inventory.JSONRoundTrip` — Equipment survives CharacterSheet serialization
- `GreymawChronicles.SprintJ.SaveLoad.ObjectCreation` — UGreymawSaveGame fields + slot name
- `GreymawChronicles.SprintJ.SaveLoad.ConversationRoundTrip` — ConversationHistory serialize/deserialize
- `GreymawChronicles.SprintJ.NPCSpawn.TagsRegistered` — "talk to marta" extracts subject
- `GreymawChronicles.SprintJ.NPCSpawn.NameConsistency` — All 3 NPC names resolve
- `GreymawChronicles.SprintJ.Inventory.DelegateExists` — OnInventoryChanged delegate exists
- `GreymawChronicles.SprintJ.WorldState.InvalidJSON` — FromJSON handles malformed input

### Sprint K Tests (12):
- `GreymawChronicles.SprintK.BugFix.StealSuccess_GivesLoot` — Loot on SuccessBranch.WorldChanges
- `GreymawChronicles.SprintK.BugFix.StealFail_NoLoot` — No inventory_add on FailureBranch or shared
- `GreymawChronicles.SprintK.BugFix.StealFail_SetsSuspicious` — Suspicious + upset on FailureBranch only
- `GreymawChronicles.SprintK.BugFix.PersuadeSuccess_SetsDisposition` — Disposition on SuccessBranch only
- `GreymawChronicles.SprintK.BugFix.PersuadeFail_NoDisposition` — No disposition on FailureBranch or shared
- `GreymawChronicles.SprintK.NPCState.TalkTransitionsToEngaged` — First talk -> engaged state
- `GreymawChronicles.SprintK.NPCState.UpsetBlocksHelpful` — Upset persists, other NPCs unaffected
- `GreymawChronicles.SprintK.TaskLoop.AcceptSetsState` — Accept -> task accepted + JSON round-trip
- `GreymawChronicles.SprintK.TaskLoop.ReportCountsClues` — 2+ clues -> resolved
- `GreymawChronicles.SprintK.TaskLoop.ReportNoClues` — 0 clues -> not resolved
- `GreymawChronicles.SprintK.SaveLoad.FeedbackDelegateFires` — OnSaveLoadFeedback broadcasts correctly
- `GreymawChronicles.SprintK.Intent.AcceptReport` — Accept/Report intents with keyword variants

### Sprint L Tests (15):
- `GreymawChronicles.SprintL.Combat.EnemyTemplateDefaults` — MakeGoblin() stats correct
- `GreymawChronicles.SprintL.Combat.InitiativeOrder` — RollInitiative sorted TurnOrder
- `GreymawChronicles.SprintL.Combat.TurnAdvancement` — AdvanceTurn cycles and wraps
- `GreymawChronicles.SprintL.Combat.VictoryCondition` — All enemies dead -> Victory
- `GreymawChronicles.SprintL.Combat.DefeatCondition` — Player dead -> Defeat
- `GreymawChronicles.SprintL.Quest.StartAndQuery` — StartQuest -> Active status
- `GreymawChronicles.SprintL.Quest.UpdateObjective` — UpdateObjective changes text
- `GreymawChronicles.SprintL.Quest.CompleteQuest` — CompleteQuest -> Completed
- `GreymawChronicles.SprintL.Scene.TrailNarrationExists` — Trail narration pool non-empty
- `GreymawChronicles.SprintL.Scene.SceneIdTracking` — WorldState scene.current round-trip
- `GreymawChronicles.SprintL.Save.MultiSlotNaming` — MakeSlotName(0/1/2) + clamping
- `GreymawChronicles.SprintL.Save.QuestLogPersistence` — Quest JSON round-trip
- `GreymawChronicles.SprintL.Audio.NullSafePlayback` — PlayEvent with null sound -> no crash
- `GreymawChronicles.SprintL.Dialogue.OptionFiltering` — State-aware option filtering
- `GreymawChronicles.SprintL.Dialogue.NodeChaining` — NextNodeId chains correctly

## Known Limitations

- Mannequin mesh may not exist in all project configurations (NPC becomes invisible tagged actor).
- Dynamic material tint is best-effort — depends on material parameter names.
- Camera subsystem switches mode but has no physical camera actors placed.
- Floor and bar use engine primitive meshes (white cubes) until tavern asset pack is imported.
- Multi-slot save available via `gc.save 0/1/2` / `gc.load 0/1/2`. Auto-save continues as `GreymawSaveSlot_0`.
- Tavern asset pack (Stylized Medieval Tavern, Baturinets) requires manual import from UE 5.4 dummy project.
- NPC interaction state has no auto-recovery from "upset" — player must face consequences.
- Combat uses manual d20 rolls (enemies lack CharacterSheets). Only goblin ambush encounter scripted.
- Dialogue trees are code-defined (Marta, Durgan). Data-driven loading from assets is future work.
- Audio subsystem is code framework only — no .wav/.ogg assets included yet.
- Scene transition is narrative-only (no level streaming or new geometry).
- Quest journal and save/load UIs are code-constructed fallbacks (no UMG Blueprint).
- Clue tracking is binary (found/not found) — no partial knowledge or quality levels.
