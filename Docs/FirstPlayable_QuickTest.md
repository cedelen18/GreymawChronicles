# First Playable - Quick Test Script

**Sprint:** H
**Time:** ~5 minutes
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

## Pass Criteria

- All 15 prompts produce narration text in the panel.
- Narration text **varies** on repeated attempts of the same prompt (narration pool).
- Fuzzy matching works: synonyms and natural phrasing resolve to correct intent.
- 3 micro-events trigger ability checks (perception DC 12, intimidation DC 13, sleight_of_hand DC 11).
- `arm wrestle` still triggers athletics DC 14 check.
- Action feedback toasts appear mid-right and auto-fade after ~3 seconds.
- Debug overlay updates state, action count, and check result correctly.
- Player visually moves during movement-intent prompts.
- Input box disables during processing and re-enables after.
- No crashes or ensure failures in the Output Log.

## Automated Tests

Run intent classifier integration tests:
```
UnrealEditor-Cmd.exe <project> -ExecCmds="Automation RunTests Greymaw.Intent" -NullRHI -NoSound
```

Tests cover:
- `Greymaw.Intent.LookAround` — look/survey/where am i -> Look intent
- `Greymaw.Intent.TalkToNPC` — talk/ask/chat + subject extraction
- `Greymaw.Intent.ChallengeAction` — arm wrestle/intimidate -> Challenge
- `Greymaw.Intent.FallbackPath` — gibberish/empty -> Unknown
- `Greymaw.Intent.MoveAction` — go/approach/sit -> Move + subject
- `Greymaw.Intent.InspectTriggersCheck` — examine/read + subject extraction

## Known Limitations

- No skeletal meshes or animations yet (montage warnings in log are expected).
- NPC actors (marta, kael, durgan) resolve to player pawn fallback (no NPC spawning yet).
- Camera subsystem switches mode but has no physical camera actors placed.
- Floor and bar use engine primitive meshes (white cubes).
- World changes (inventory_add for mysterious coin) are logged but not persisted yet.
