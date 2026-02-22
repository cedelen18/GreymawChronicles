# First Playable - Quick Test Script

**Sprint:** G.1
**Time:** ~3 minutes
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
| Narration Panel | Bottom (40, 620) | Typewriter text area |
| Input Box | Bottom (40, 790) | Player text input |

## Prompt Sequence

Type each prompt into the input box and press Enter. Verify the expected response.

### 1. `look around`
- **Narration:** Describes the Thornhaven taproom (Marta, Kael, Durgan, hearth, quest board).
- **Debug:** `[DM] Processing...` flashes, then `[DM] Idle`. `[Actions] 1 queued`.
- **Camera:** Establishing shot.

### 2. `talk to marta`
- **Narration:** Marta describes missing villagers, offers 50 gold quest.
- **Debug:** `[Actions] 3 queued` (move + talk_gesture + nod).
- **Action:** Player lerps toward bar position (320, 0, 100).
- **Camera:** Close mode (keyword `talk`).

### 3. `walk to bar`
- **Narration:** Floorboards creak, regulars size you up.
- **Debug:** `[Actions] 3 queued` (move + wait + idle).
- **Action:** Player lerps to (350, 0, 100).
- **Camera:** Medium mode (keyword `move`).

### 4. `arm wrestle`
- **Narration:** Kael slams elbow on table.
- **Debug:** `[Check] athletics | Roll:X + Mod:3 = Y vs DC 14  PASS/FAIL`.
- **Branch:** Success = "slam Kael's hand" / Failure = "Kael overpowers you".
- **Color:** Green text on PASS, red on FAIL. `CRIT!` or `FUMBLE!` on nat 20/1.

### 5. `talk to kael`
- **Narration:** Kael offers to join you on the trail north.
- **Debug:** `[Actions] 1 queued`.

### 6. `durgan`
- **Narration:** Old Durgan describes the Greymaw ("a mouth in the earth").
- **Debug:** `[Actions] 1 queued`.

### 7. `quest board`
- **Narration:** Weathered parchment about missing villagers, 50 gold reward.
- **Debug:** `[Actions] 1 queued`.

### 8. `hello` (catch-all)
- **Narration:** Reminder of available NPCs and objects in the taproom.
- **Debug:** `[Actions] 1 queued`.

## Pass Criteria

- All 8 prompts produce narration text in the panel.
- Debug overlay updates state (`Processing`/`Idle`), action count, and check result.
- `arm wrestle` triggers a visible dice check with roll/modifier/DC/result.
- Player visually moves during `talk to marta` and `walk to bar`.
- No crashes or ensure failures in the Output Log.

## Known Limitations

- No skeletal meshes or animations yet (montage warnings in log are expected).
- NPC actors (marta, kael, durgan) resolve to player pawn fallback (no NPC spawning yet).
- Camera subsystem switches mode but has no physical camera actors placed.
- Floor and bar use engine primitive meshes (white cubes).
