# Sprint K — NPC State Machine, Task Mini-Loop, Save/Load UX, Camera Polish

**Date:** 2026-02-22
**Branch:** master

---

## Summary

Sprint K moves the project from tech demo to true first-playable vertical slice. Three bugs in outcome-conditional WorldChanges are fixed, NPCs now track interaction state (neutral/engaged/helpful/upset), a complete tavern investigation mini-loop (accept task, gather clues, report) is playable, save/load events show feedback toasts, camera timing is polished with null-safe fallbacks, and 12 new tests bring the total to 46.

---

## Bugs Fixed

| # | Bug | Root Cause | Fix |
|---|-----|-----------|-----|
| 1 | Steal gives loot regardless of check outcome | `FDMOutcomeBranch` had no `WorldChanges` field; loot on shared array | Added `WorldChanges` to `FDMOutcomeBranch`; moved loot to `SuccessBranch.WorldChanges` |
| 2 | Persuade sets disposition unconditionally | Same root cause; disposition changes on shared array | Moved disposition changes to `SuccessBranch.WorldChanges` |
| 3 | Coin micro-event dead code | `CoinChange` constructed but never added to any array | Added to `SuccessBranch.WorldChanges` |

---

## New Files

| File | Purpose |
|------|---------|
| `Private/Tests/GCSprintKTests.cpp` | 12 new tests covering all Sprint K features |
| `Docs/SprintK_Notes.md` | This file |

## Modified Files

| File | Changes |
|------|---------|
| `Public/DungeonMaster/DMTypes.h` | +WorldChanges on FDMOutcomeBranch |
| `Public/DungeonMaster/DMBrainSubsystem.h` | +GetNPCInteractionState, +TransitionNPCState |
| `Private/DungeonMaster/DMBrainSubsystem.cpp` | Bug fixes, NPC state machine, Accept/Report handlers, clue tracking, set_state handler |
| `Public/DungeonMaster/DMIntentClassifier.h` | +Accept, +Report intent enums |
| `Private/DungeonMaster/DMIntentClassifier.cpp` | Keyword rules for Accept/Report |
| `Private/DungeonMaster/DMNarrationPool.cpp` | +10 new narration slots (~30 variants) |
| `Public/Core/GCGameInstance.h` | +OnSaveLoadFeedback delegate, +console command members |
| `Private/Core/GCGameInstance.cpp` | Feedback broadcasts on save/load, gc.save/gc.load console commands |
| `Public/UI/GCActionFeedbackWidget.h` | +HandleSaveLoadFeedback |
| `Private/UI/GCActionFeedbackWidget.cpp` | Bind save/load feedback delegate, show dim toast |
| `Private/ActionSystem/ActionDirectorSubsystem.cpp` | Timing adjustments, null-safe camera fallback |
| `Docs/FirstPlayable_QuickTest.md` | Sprint K prompts, updated pass criteria and test list |

---

## Phase 1: Outcome-Conditional WorldChanges (Bug Fix)

**Root cause:** `FDMOutcomeBranch` lacked a `WorldChanges` array, so all world changes were placed on the shared `FDMResponse::WorldChanges` and applied unconditionally.

**Fix:** Added `TArray<FDMWorldChange> WorldChanges` to `FDMOutcomeBranch`. Updated `ResolveParsedResponse()`:
1. On check success: apply `SuccessBranch.WorldChanges`
2. On check failure: apply `FailureBranch.WorldChanges`
3. Always: apply shared `Parsed.WorldChanges` (unconditional changes)

Handler-specific fixes:
- **Steal:** Loot (`inventory_add`) on `SuccessBranch`, suspicious + upset on `FailureBranch`
- **Persuade:** Disposition + helpful state on `SuccessBranch` only
- **Coin:** `inventory_add mysterious_coin` on `SuccessBranch` (was never added to any array)

---

## Phase 2: NPC Interaction State Machine

Per-NPC state tracked via existing `DMWorldStateSubsystem` (category: `npc_interaction_state`).

**States:** neutral (default) -> engaged -> helpful / upset

**Helper methods on DMBrainSubsystem:**
- `GetNPCInteractionState(NPCName)` — returns "neutral" if empty
- `TransitionNPCState(NPCName, NewState)` — sets WorldState

**State transitions:**
| Action | From | To |
|--------|------|----|
| Talk to NPC (first time) | neutral | engaged |
| Persuade NPC (pass) | any | helpful |
| Steal from NPC (fail) | any | upset |
| Order food | any | helpful |

**Narration routing:** Talk handler checks interaction state and routes to state-specific narration slots (e.g., `talk_marta_engaged`, `talk_durgan_suspicious`).

---

## Phase 3: Tavern Task Mini-Loop

One complete gameplay loop: accept task -> gather clues -> report back -> resolution.

**New intents:** Accept (accept, agree, volunteer) and Report (report, share findings, inform)

**Task tracking via WorldState:**
- `task:tavern_investigation` = accepted / resolved
- `task_clue:overheard_snippet` = true (Listen success)
- `task_clue:durgan_lore` = true (Persuade Durgan success)
- `task_clue:board_notice` = true (Inspect quest board)

**New WorldChange type:** `set_state` — generic state setter with `category:key` format splitting.

**Report handler branches:**
- 0 clues: "You haven't learned anything yet."
- 1 clue: Partial resolution (modest reward narration)
- 2+ clues: Full resolution, task marked resolved

---

## Phase 4: Save/Load UX Feedback

- `FOnSaveLoadFeedback` delegate on `GCGameInstance`
- Auto-save broadcasts "Autosaved" (dim grey toast, 2s)
- Auto-load broadcasts "Loaded save from <timestamp>" (dim grey toast, 2s)
- Console commands: `gc.save` and `gc.load` for manual save/load
- `GCActionFeedbackWidget` binds delegate via `HandleSaveLoadFeedback()`

---

## Phase 5: Camera/Action Timing Polish

| Parameter | Before | After | Rationale |
|-----------|--------|-------|-----------|
| Sequence safety timeout | 15s | 12s | Faster stuck recovery |
| Post-sequence camera blend | 1.2s | 1.8s | Smoother return to establishing |
| Inspect board delay | 1.5s | 0.6s | Remove dead air |

**Null-safe camera fallback:** When `ResolveActor()` returns nullptr for camera target, falls back to player pawn with Warning log (not Error). Prevents camera freezing on unresolved actors.

**Redundant camera skip:** Camera switches are skipped when mode + focus actor are identical to last switch (prevents jitter on consecutive same-target actions).

---

## Phase 6: Tests

**12 new tests** (46 total: 21 Sprint H + 10 Sprint I + 10 Sprint J + 12 Sprint K - 7 superseded = 46)

| # | Test | Validates |
|---|------|-----------|
| 1 | BugFix.StealSuccess_GivesLoot | Loot on SuccessBranch.WorldChanges |
| 2 | BugFix.StealFail_NoLoot | No inventory_add on FailureBranch or shared |
| 3 | BugFix.StealFail_SetsSuspicious | Suspicious + upset on FailureBranch only |
| 4 | BugFix.PersuadeSuccess_SetsDisposition | Disposition on SuccessBranch only |
| 5 | BugFix.PersuadeFail_NoDisposition | No disposition on FailureBranch or shared |
| 6 | NPCState.TalkTransitionsToEngaged | First talk -> npc_interaction_state = engaged |
| 7 | NPCState.UpsetBlocksHelpful | Upset persists, other NPCs unaffected |
| 8 | TaskLoop.AcceptSetsState | Accept -> task:tavern_investigation = accepted |
| 9 | TaskLoop.ReportCountsClues | 2+ clues -> resolved |
| 10 | TaskLoop.ReportNoClues | 0 clues -> not resolved |
| 11 | SaveLoad.FeedbackDelegateFires | OnSaveLoadFeedback broadcasts correctly |
| 12 | Intent.AcceptReport | Accept/Report intents classify with variants |

---

## Known Limitations

- NPC state machine has no auto-recovery path from "upset" (intentional — player must deal with consequences).
- Task mini-loop is single-task only (tavern_investigation). Framework supports more but only one is scripted.
- Console commands `gc.save`/`gc.load` are development-only (no in-game UI).
- Clue tracking is binary (found/not found) — no partial knowledge or clue quality.
