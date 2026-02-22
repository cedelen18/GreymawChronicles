# Sprint I — Interaction Depth, Polish, UX, Tests, Docs

**Date:** 2026-02-22
**Branch:** master

---

## Summary

Sprint I deepens the first-playable tavern experience with 6 new interaction intents, an NPC disposition system that tracks consequences within a session, smoother action pacing, better check readability, safety guards against stuck states, and 10 new tests bringing the total to 31.

---

## New Files

| File | Purpose |
|------|---------|
| `Public/DungeonMaster/DMWorldStateSubsystem.h` | Session-scoped key-value state store (NPC dispositions, flags) |
| `Private/DungeonMaster/DMWorldStateSubsystem.cpp` | Implementation: SetState, GetState, HasState, ClearState over nested TMap |
| `Private/Tests/GCSprintIIntegrationTest.cpp` | 10 new tests for intents, world state, narration slots, regression |
| `Docs/SprintI_Notes.md` | This file |

## Modified Files

| File | Changes |
|------|---------|
| `Public/DungeonMaster/DMIntentClassifier.h` | +6 enum values (Order, Steal, Listen, Persuade, Rest, Gamble) |
| `Private/DungeonMaster/DMIntentClassifier.cpp` | +6 keyword rules, +13 known subjects |
| `Public/DungeonMaster/DMBrainSubsystem.h` | +WorldState ptr, +ProcessingTimeoutHandle, +DebounceHandle |
| `Private/DungeonMaster/DMBrainSubsystem.cpp` | +6 intent handlers, extended ApplyWorldChanges, disposition-aware Talk, timeout/debounce in SetProcessingState, deception skill mapping |
| `Private/DungeonMaster/DMNarrationPool.cpp` | +16 narration slots (48 variant lines) |
| `Public/ActionSystem/ActionDirectorSubsystem.h` | +SequenceTimeoutHandle, +LastCameraMode, +LastCameraFocusActor, +ForceCompleteSequence |
| `Private/ActionSystem/ActionDirectorSubsystem.cpp` | Variable blend times, skip redundant camera, sequence timeout (15s), post-sequence camera reset, interrupted sequence recovery |
| `Public/UI/GCActionFeedbackWidget.h` | +PushToastWithDuration, MaxToasts 4->6, +CheckToastDuration |
| `Private/UI/GCActionFeedbackWidget.cpp` | Multi-line check feedback (3 toasts: header/roll/result) |
| `Public/UI/GCPlayerInputWidget.h` | +InputRecoveryHandle |
| `Private/UI/GCPlayerInputWidget.cpp` | 20s input recovery timeout |
| `Docs/FirstPlayable_QuickTest.md` | +prompts 16-21, updated pass criteria, updated test counts |

---

## New Intents

| Intent | Keywords | Confidence |
|--------|----------|------------|
| Order | order, buy, purchase, request, serve, another round | 0.90 |
| Steal | steal, pickpocket, pilfer, swipe, snatch, pocket, filch, lift | 0.95 |
| Listen | listen, eavesdrop, overhear, hear, spy | 0.90 |
| Persuade | persuade, convince, bribe, coax, entreat, charm, flatter, flirt | 0.90 |
| Rest | rest, sleep, meditate, relax, nap, sit down, take a break | 0.85 |
| Gamble | gamble, bet, wager, dice, cards, game, play dice, play cards | 0.90 |

---

## NPC World State System

`UDMWorldStateSubsystem` — a `UGameInstanceSubsystem` providing session-scoped key-value storage.

**Storage:** `Category -> Key -> Value` (nested TMap)
**Example:** `"npc_disposition" -> "marta" -> "friendly"`

### Stateful Consequences

1. **Ordering food from Marta** sets `marta=friendly` -> Talk uses warm narration
2. **Failed steal on Marta** sets `marta=suspicious` -> Talk becomes curt, orders refused
3. **Successful persuasion of Durgan** sets `durgan=open` -> Talk reveals extra Greymaw lore
4. **Successful persuasion of Marta** sets `marta=trusting` -> Talk uses friendly slot

---

## Pacing Improvements

- **Variable camera blend times:** Establishing=1.0s, Medium=0.6s, Close=0.35s, Combat=0.5s
- **Redundant camera skip:** Same mode + same focus actor = no camera switch
- **Post-sequence camera reset:** Blends back to Establishing over 1.2s after sequence ends

## Safety Guards

- **Sequence timeout (15s):** ActionDirector force-completes stuck sequences
- **Processing timeout (20s):** DMBrain force-resets processing state + broadcasts timeout narration
- **Input debounce (0.3s):** Prevents double-submit after processing completes
- **Input recovery (20s):** PlayerInputWidget force-restores input if processing never completes
- **Interrupted sequence recovery:** New ExecuteDMActions force-completes previous running sequence

## UX Feedback

- **Multi-line check toasts:** 3-line breakdown (gold header, white roll details, green/red result)
- **MaxToasts:** 4 -> 6
- **Check toast duration:** 3s -> 5s
- **Crit/Fumble tags:** Appended to roll line

---

## Test Coverage

**31 total tests** (21 existing + 10 new)

Sprint I tests:
1. Intent.Order — keyword classification
2. Intent.Steal — keyword + subject extraction
3. Intent.Listen — keyword classification
4. Intent.Persuade — keyword + subject extraction
5. Intent.Rest — keyword classification
6. Intent.Gamble — keyword classification
7. WorldState.SetGet — full CRUD round-trip
8. WorldState.DispositionAffectsNarration — slot differentiation
9. NarrationPool.NewSlots — all 16 slots populated
10. Timeout.ProcessingRecovery — existing intent regression check

---

## Known Limitations

- NPC state is session-scoped only (no save/load persistence yet)
- NPC actors still resolve to player pawn fallback (no NPC spawning)
- No skeletal meshes or animations (montage warnings expected)
- Camera subsystem switches mode but has no physical camera actors
- Steal world change for inventory_add is logged but not tracked in inventory UI
