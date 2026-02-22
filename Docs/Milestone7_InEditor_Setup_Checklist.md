# Milestone 7 In-Editor Setup Checklist (HUD/Input Bootstrap)

Use this once per fresh project checkout to ensure visible gameplay UI appears when pressing **Play**.

## 1) Create the persistent level

1. In Content Browser create folder: `/Game/Maps` (if missing).
2. Create a new level named `L_Persistent`.
3. Open `L_Persistent`.
4. Add a `PlayerStart` actor in the level.
5. Save all.

## 2) Set project defaults

1. Open **Project Settings → Maps & Modes**.
2. Set:
   - **Editor Startup Map** = `/Game/Maps/L_Persistent`
   - **Game Default Map** = `/Game/Maps/L_Persistent`
   - **Default GameMode** = `GCGameMode` (from GreymawChronicles)
3. Save settings.

> Repo default ini currently points to `/Engine/Maps/Entry` as a safe non-breaking fallback for new clones.
> After `L_Persistent` exists, switch both defaults to `/Game/Maps/L_Persistent`.

## 3) Verify HUD/input bootstrap in PIE

1. Open `L_Persistent`.
2. Press **Play**.
3. Confirm these are visible:
   - DM text panel area
   - Player input text box (hint: "What do you do?")
   - Submit button area
4. Type a short prompt and press Enter.
5. Confirm input widget disables briefly while processing and then re-enables.

## Troubleshooting

- If UI does not appear, verify the level is running with `GCGameMode`.
- Ensure no Blueprint override removes the viewport widget at BeginPlay.
- Re-run PIE: the GameMode now retries HUD spawn if PlayerController appears late.
