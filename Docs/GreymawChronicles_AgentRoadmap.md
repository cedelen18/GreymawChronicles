# Greymaw Chronicles — Agent Team Implementation Roadmap
## For Claude Code Orchestrator + Agent Teams

**Project:** GreymawChronicles
**Location:** `C:\UE5Projects\GreymawChronicles`
**Engine:** Unreal Engine 5.7.3
**Reference Document:** `GreymawChronicles_GDD.md` (read this first for full context)

---

## How to Use This Document

This roadmap is structured for a **Claude Code orchestrator** that assigns tasks to agent teams.

**Workflow:**
1. Read this file and the GDD for full context
2. Find the next task where `Status: NOT_STARTED` and all `Dependencies` have `Status: COMPLETE`
3. Assign the task to an agent team with the task's instructions
4. When the agent team completes the task and passes acceptance criteria, update the status to `COMPLETE`
5. Repeat until all milestones are complete

**Status Values:** `NOT_STARTED` | `IN_PROGRESS` | `COMPLETE` | `BLOCKED`

**Rules:**
- Always read the GDD for full context on any system being built
- Never skip acceptance criteria
- Tasks within a milestone can sometimes be parallelized if they don't depend on each other
- Each task lists the specific files to create/modify
- When a task says "see GDD section X" — the agent MUST read that section before coding

---

## Milestone 1: Project Scaffolding
**Status:** NOT_STARTED | **Dependencies:** None

### Task 1.1: Create UE5 C++ Project
**Status:** NOT_STARTED | **Deps:** None | **Team:** infrastructure
**Instructions:** Create a new UE5 C++ project using the Third Person template. Name: `GreymawChronicles`. Location: `C:\UE5Projects\`.
**Output:** `C:\UE5Projects\GreymawChronicles\GreymawChronicles.uproject` and all default source files.
**Acceptance:**
- [ ] Project exists and compiles with zero errors
- [ ] Editor launches and loads default map

### Task 1.2: Establish Module Directory Structure
**Status:** NOT_STARTED | **Deps:** 1.1 | **Team:** infrastructure
**Instructions:** Create all source directories and placeholder class files as defined in GDD Section 6 "Module Structure." Each placeholder file should have a proper UCLASS declaration with the parent class specified in the GDD. Update `Build.cs` with dependencies: Core, CoreUObject, Engine, InputCore, EnhancedInput, HTTP, Json, JsonUtilities, UMG, Slate, SlateCore, SQLiteCore.

**Directories:** Core, Character, DungeonMaster, Ollama, RulesEngine, SceneSystem, ActionSystem, UI, Inventory, SaveSystem

**Key placeholder classes (see GDD for full list):**
- `Core/GCGameInstance.h/cpp` — UGameInstance
- `Core/GCGameMode.h/cpp` — AGameModeBase
- `DungeonMaster/DMBrainSubsystem.h/cpp` — UGameInstanceSubsystem
- `Ollama/OllamaSubsystem.h/cpp` — UGameInstanceSubsystem
- `SaveSystem/GCSaveSubsystem.h/cpp` — UGameInstanceSubsystem
- (All other classes from GDD Section 6)

**Acceptance:**
- [ ] All directories and placeholder files exist
- [ ] Build.cs has all module dependencies
- [ ] Project compiles with zero errors

### Task 1.3: Configure Git Repository
**Status:** NOT_STARTED | **Deps:** 1.1 | **Team:** infrastructure
**Instructions:** Init Git repo at project root. Create `.gitignore` (exclude Binaries, Intermediate, Saved, DerivedDataCache, .vs). Create `.gitattributes` for Git LFS (`.uasset`, `.umap`, `.png`, `.jpg`, `.wav`, `.mp3`, `.fbx`). Initial commit.
**Acceptance:**
- [ ] Git repo initialized with proper ignore/LFS config
- [ ] Initial commit made

### Task 1.4: Create Content Directory Structure
**Status:** NOT_STARTED | **Deps:** 1.1 | **Team:** infrastructure
**Instructions:** Create all Content subdirectories per GDD Section 6 "Content Directory Structure." Create empty `L_Persistent` and `L_MainMenu` level maps.
**Acceptance:**
- [ ] All Content directories exist
- [ ] Empty levels created
- [ ] Project compiles and editor launches

### Task 1.5: Configure Game Instance and Game Mode
**Status:** NOT_STARTED | **Deps:** 1.2 | **Team:** infrastructure
**Instructions:** Implement GCGameInstance to register subsystems (Ollama, DMBrain, Save). Implement GCGameMode with basic setup. Update DefaultEngine.ini and DefaultGame.ini to use these classes. Set L_Persistent as default map.
**Acceptance:**
- [ ] Game launches with GCGameInstance and GCGameMode
- [ ] All three subsystems initialize (verify with log output)
- [ ] L_Persistent loads as default map

---

## Milestone 2: Ollama Integration
**Status:** NOT_STARTED | **Dependencies:** Milestone 1

### Task 2.1: Implement OllamaSubsystem Core
**Status:** NOT_STARTED | **Deps:** 1.5 | **Team:** ai-integration
**Instructions:** Implement UOllamaSubsystem as a UGameInstanceSubsystem managing HTTP communication with Ollama at `http://localhost:11434`. Use UE5's FHttpModule (IHttpRequest/IHttpResponse). All requests must be async.

Key functions:
- `Initialize()` — health check via GET /api/tags
- `IsOllamaAvailable()` — connection status
- `GetAvailableModels()` — returns installed models
- `SendGenerateRequest(FString Model, FString Prompt, FOllamaRequestOptions Options, FOnOllamaComplete Callback)` — async POST to /api/generate
- Error handling: 15s timeout, connection refused, invalid response

See GDD Section 3 "Ollama Integration Details" for request format and parameters.
**Acceptance:**
- [ ] Health check works on startup
- [ ] Test prompt to qwen2.5:32b returns a response
- [ ] Test prompt to llama3.1:8b returns a response
- [ ] Graceful failure if Ollama is not running

### Task 2.2: Implement OllamaRequest Wrapper
**Status:** NOT_STARTED | **Deps:** 2.1 | **Team:** ai-integration
**Instructions:** Implement UOllamaRequest wrapping individual HTTP requests. Tracks: model, prompt, options, state (pending/in_progress/completed/failed/timed_out), response text, retry count (max 2), priority (CRITICAL/HIGH/NORMAL/LOW), timestamp. Completion delegate: `FOnOllamaRequestComplete(bool bSuccess, FString ResponseText, float LatencySeconds)`.
**Acceptance:**
- [ ] Request lifecycle tracking works
- [ ] Retry logic retries up to 2 times on failure
- [ ] Completion delegate fires with correct data

### Task 2.3: Implement Request Priority Queue
**Status:** NOT_STARTED | **Deps:** 2.2 | **Team:** ai-integration
**Instructions:** Implement UOllamaRequestQueue. Priority: CRITICAL > HIGH > NORMAL > LOW. One request per model at a time. CRITICAL preempts lower priority. FIFO within same priority. Update OllamaSubsystem to use this queue.
**Acceptance:**
- [ ] Requests process in priority order
- [ ] One request per model in-flight
- [ ] CRITICAL preempts lower priority
- [ ] Test: queue 3 requests at different priorities, verify order

### Task 2.4: Startup Health Check UI
**Status:** NOT_STARTED | **Deps:** 2.1 | **Team:** ai-integration
**Instructions:** On game startup, check Ollama availability and model presence. Show clear error if Ollama not running or models missing (with exact `ollama pull` commands). Proceed to main menu if all good.
**Acceptance:**
- [ ] Error shown if Ollama unavailable
- [ ] Missing model names and pull commands shown
- [ ] Proceeds to main menu when healthy
- [ ] No crash on Ollama unavailable

---

## Milestone 3: DM Brain Core
**Status:** NOT_STARTED | **Dependencies:** Milestone 2

### Task 3.1: Implement GCCharacterSheet Data Structure
**Status:** NOT_STARTED | **Deps:** 1.2 | **Team:** gameplay
**Instructions:** Implement UGCCharacterSheet as UObject containing full 5e stat block. See GDD Section 2 for class stat blocks. Include: name, race, class, level, ability scores, HP, AC, proficiency bonus, saving throws, skills, class features, equipment, spell slots, spells, cantrips. Utility: GetAbilityModifier(), GetSkillModifier(), GetSavingThrowModifier(), ToCompactJSON(), FromJSON(). Define enums: EGCRace, EGCClass, EGCAbility, EGCSkill.
**Acceptance:**
- [ ] All 5e stats represented
- [ ] Modifier calculations correct (STR 16 → +3)
- [ ] Proficiency bonus added when proficient
- [ ] ToCompactJSON() and FromJSON() round-trip correctly

### Task 3.2: Create Pre-built Character Sheet Data Assets
**Status:** NOT_STARTED | **Deps:** 3.1 | **Team:** gameplay
**Instructions:** Create 13 UGCCharacterSheet Data Assets: 12 race/class combos (3 races × 4 classes) + Kael. All Level 3 with standard array ability scores, racial bonuses, class features, and spells. Store in Content/Data/CharacterSheets/. Use GDD Section 2 and Section 4 for exact stat blocks.
**Acceptance:**
- [ ] All 13 data assets load without errors
- [ ] Each has complete valid 5e stat block
- [ ] Kael's sheet matches GDD exactly

### Task 3.3: Implement DMPromptBuilder
**Status:** NOT_STARTED | **Deps:** 3.1, 2.1 | **Team:** ai-core
**Instructions:** Implement UDMPromptBuilder. Assembles the full system prompt from 7 sections (see GDD Section 3 "Prompt Architecture"): DM personality, action vocabulary, output format schema, current scene context, character sheets, adventure state, conversation history. Key functions: BuildDMPrompt() and BuildCompanionCombatPrompt(). Load prompt templates from text assets or hardcoded constants.
**Acceptance:**
- [ ] BuildDMPrompt() assembles complete prompt with all 7 sections
- [ ] Dynamic sections inject data correctly
- [ ] BuildCompanionCombatPrompt() produces valid combat prompt
- [ ] Test: build with sample data, verify all sections present

### Task 3.4: Implement DMResponseParser
**Status:** NOT_STARTED | **Deps:** 3.3 | **Team:** ai-core
**Instructions:** Implement UDMResponseParser. Parse JSON responses from LLM into FDMResponse structs. Define all response structs: FDMAction, FDMWorldChange, FDMCheckRequired, FDMOutcomeBranch, FDMSceneChange, FDMResponse. Include JSON repair layer (fix missing brackets, trailing commas). Validate actions against approved animation whitelist. See GDD Section 3 "Output Format Specification" for the schema.
**Acceptance:**
- [ ] Valid JSON parses correctly into structs
- [ ] JSON repair fixes common issues
- [ ] Unknown animation names flagged by validation
- [ ] Test with GDD example JSON
- [ ] bValid=false with descriptive error on total failure

### Task 3.5: Implement DMConversationHistory
**Status:** NOT_STARTED | **Deps:** None | **Team:** ai-core
**Instructions:** Circular buffer storing last 15 conversation exchanges. Each FDMExchange: PlayerInput, DMResponseSummary, Timestamp, SceneID. Functions: AddExchange(), GetRecentExchanges(), ToPromptString(), Clear(), SerializeToJSON(), DeserializeFromJSON().
**Acceptance:**
- [ ] Buffer stores up to capacity, drops oldest
- [ ] ToPromptString() produces readable LLM-ready text
- [ ] Serialization round-trips correctly

### Task 3.6: Implement DMBrainSubsystem Core Loop
**Status:** NOT_STARTED | **Deps:** 3.3, 3.4, 3.5, 2.1 | **Team:** ai-core
**Instructions:** Main orchestrator UGameInstanceSubsystem. ProcessPlayerInput() → build prompt → send to Ollama → parse response → fire delegates. Delegates: FOnDMNarration, FOnDMActionsReady, FOnDMSceneChangeRequested, FOnDMCheckRequired, FOnDMCompanionReaction. Also: UpdateSceneContext(), UpdateAdventureState(). See GDD Section 3 for full architecture.
**Acceptance:**
- [ ] ProcessPlayerInput() sends prompt to Ollama and gets response
- [ ] Response parses into FDMResponse
- [ ] Narration delegate fires with text
- [ ] Conversation history updated each exchange
- [ ] Test: "I look around the room" → valid DM response received and parsed

---

## Milestone 4: 5e Rules Engine
**Status:** NOT_STARTED | **Dependencies:** Task 3.1

### Task 4.1: Implement DiceRoller
**Status:** NOT_STARTED | **Deps:** None | **Team:** gameplay
**Instructions:** UDiceRoller — all dice rolling. RollD20(), RollDice(NumDice, DieSize), RollD20WithAdvantage(), RollD20WithDisadvantage(), RollDiceDetailed(). Use FMath::RandRange(). Fire delegate with roll details for UI.
**Acceptance:**
- [ ] Correct value ranges
- [ ] Advantage/disadvantage correct
- [ ] Roll delegate fires

### Task 4.2: Implement AbilityCheck
**Status:** NOT_STARTED | **Deps:** 4.1, 3.1 | **Team:** gameplay
**Instructions:** Resolves skill checks and saving throws. ResolveAbilityCheck(Sheet, Skill, DC, Advantage, Disadvantage) → FAbilityCheckResult (roll, modifier, total, DC, success, crit success, crit fail). ResolveSavingThrow() same pattern. Gets modifier from sheet, rolls d20, compares to DC.
**Acceptance:**
- [ ] Correct modifier from sheet
- [ ] DC comparison correct
- [ ] Nat 20/1 flagged
- [ ] Advantage/disadvantage works

### Task 4.3: Implement CombatResolver
**Status:** NOT_STARTED | **Deps:** 4.1, 3.1 | **Team:** gameplay
**Instructions:** Attack rolls, damage, combat flow. ResolveAttackRoll(Attacker, Weapon, Defender, Advantage, Disadvantage) → FAttackResult. CalculateDamage(Weapon, Modifier, Critical). ApplyDamage(Target, Damage). ResolveInitiative(Combatants). Critical hits double damage dice. See GDD Section 4 for weapon/enemy stats.
**Acceptance:**
- [ ] Attack roll uses correct modifier vs AC
- [ ] Critical hits double dice
- [ ] Damage calculation correct for all MVP weapons
- [ ] Initiative sorting works

### Task 4.4: Implement SpellSystem
**Status:** NOT_STARTED | **Deps:** 4.1, 4.3, 3.1 | **Team:** gameplay
**Instructions:** Spell slot tracking and resolution. HasSpellSlot(), ExpendSpellSlot(), ResolveSpellAttack(), GetSpellSaveDC(), GetSpellAttackModifier(). Define all MVP spells as data (see GDD Section 2 for spell lists). Cantrips: Fire Bolt, Sacred Flame, Guidance, Spare the Dying, Mage Hand, Prestidigitation. Level 1: Magic Missile, Shield, Thunderwave, Cure Wounds, Healing Word, Guiding Bolt. Level 2: Scorching Ray, Misty Step, Spiritual Weapon, Aid.
**Acceptance:**
- [ ] Spell slot tracking works
- [ ] Spell save DC correct per class
- [ ] All MVP spells defined
- [ ] Cantrips don't consume slots

### Task 4.5: Wire Rules Engine into DM Brain
**Status:** NOT_STARTED | **Deps:** 4.1-4.4, 3.6 | **Team:** ai-core
**Instructions:** Update DMBrainSubsystem: when FDMResponse.bCheckRequired is true, extract check details, call appropriate rules engine function, fire dice roll delegate, select success/failure branch based on result, fire narration and actions delegates. When no check required, fire narration/actions directly.
**Acceptance:**
- [ ] Check requests resolved by rules engine
- [ ] Correct outcome branch selected
- [ ] Damage/healing updates character sheets
- [ ] Test end-to-end: player input → DM → dice roll → outcome

---

## Milestone 5: Basic UI
**Status:** NOT_STARTED | **Dependencies:** Milestone 3

### Task 5.1: Implement DM Text Panel
**Status:** NOT_STARTED | **Deps:** 3.6 | **Team:** ui
**Instructions:** UUserWidget at bottom 20% of screen. Semi-transparent dark bg (#000000 80% opacity). Typewriter text effect (~30 chars/sec). Scrollable. Auto-scroll to bottom. Warm off-white text (#F0E6D3). Binds to FOnDMNarration delegate.
**Acceptance:**
- [ ] Renders at bottom of screen
- [ ] Typewriter effect works
- [ ] Scrollable with auto-scroll
- [ ] Displays DM responses

### Task 5.2: Implement Player Input Box
**Status:** NOT_STARTED | **Deps:** 5.1 | **Team:** ui
**Instructions:** Text input below DM panel. Placeholder: "What do you do?" Enter sends to DMBrainSubsystem::ProcessPlayerInput(), clears input, disables until DM responds. Escape unfocuses. Shows "..." while DM processes.
**Acceptance:**
- [ ] Text input works, Enter submits
- [ ] Disabled during DM processing
- [ ] Re-enables after response
- [ ] Sends text to DMBrainSubsystem

### Task 5.3: Implement HUD Overlay
**Status:** NOT_STARTED | **Deps:** 3.1 | **Team:** ui
**Instructions:** Top-left: player portrait area (class icon, name, level, HP bar green→yellow→red). Below: companion same format. Updates from character sheets.
**Acceptance:**
- [ ] Player and companion info displays correctly
- [ ] HP bars update in real-time
- [ ] Compact design

### Task 5.4: Implement Main Menu
**Status:** NOT_STARTED | **Deps:** 1.4 | **Team:** ui
**Instructions:** Title screen: "Greymaw Chronicles" with subtitle. Buttons: New Adventure, Continue (greyed if no save), Settings, Quit. New Adventure → character selection screen (pick race and class from dropdowns/buttons, name input, preview of stat block). Start button → loads persistent level and begins the adventure.
**Acceptance:**
- [ ] Title and buttons render
- [ ] New Adventure opens character selection
- [ ] Race/class selection works
- [ ] Start begins the game

### Task 5.5: Implement Settings Menu
**Status:** NOT_STARTED | **Deps:** 1.5 | **Team:** ui
**Instructions:** Graphics: quality preset dropdown (Low/Medium/High/Ultra), resolution, window mode, VSync, upscaling method + quality. Audio: master, music, SFX, ambient volume sliders. Apply button saves to Scalability.ini/GameUserSettings.ini. Use UE5 scalability system.
**Acceptance:**
- [ ] All settings display and are adjustable
- [ ] Apply saves settings and they take effect
- [ ] Settings persist between sessions

---

## Milestone 6: The Talking Demo
**Status:** NOT_STARTED | **Dependencies:** Milestones 3, 4, 5

### Task 6.1: Wire UI to DM Brain End-to-End
**Status:** NOT_STARTED | **Deps:** 5.1, 5.2, 5.3, 4.5 | **Team:** integration
**Instructions:** Connect all the pieces: Player types in input box → text goes to DMBrainSubsystem → prompt sent to Ollama → response parsed → narration shown in DM text panel → dice rolls display if needed → outcome narration shown. This is the first time a player can "play D&D" with the AI. No 3D scene yet — just text UI on a placeholder background.

Wire up:
1. PlayerInputBox → DMBrainSubsystem.ProcessPlayerInput()
2. DMBrainSubsystem.FOnDMNarration → DMTextPanel
3. DMBrainSubsystem.FOnDMCheckRequired → (log for now, dice display comes later)
4. HUD overlay shows player/companion HP

Set up a test scenario: hard-code the adventure state to "tavern_arrival", set the scene context to the tavern, spawn Marta/Kael/Durgan as NPC data in the context. The player should be able to have a full conversation with the DM about the tavern scene.

**Acceptance:**
- [ ] Player types "I walk up to the bar and talk to the innkeeper" → DM narrates the interaction
- [ ] Player types "I ask about the disappearances" → DM responds in character as Marta
- [ ] Player types "I try to arm wrestle Kael" → DM requests a check, rules engine resolves it
- [ ] Conversation history builds correctly across multiple exchanges
- [ ] HUD shows player and companion stats
- [ ] No crashes on malformed LLM responses (fallback handling works)

**THIS IS THE FIRST PLAYTEST MILESTONE. Spend time here testing the DM's responses, tuning prompts, and verifying the feel of the interaction.**
# Greymaw Chronicles — Agent Roadmap Part 2 (Milestones 7-22)
## Continuation of Agent Team Implementation Roadmap

---

## Milestone 7: Scene System
**Status:** NOT_STARTED | **Dependencies:** Milestone 1

### Task 7.1: Implement SceneTemplate Data Asset
**Status:** NOT_STARTED | **Deps:** 1.2 | **Team:** scene
**Instructions:** Implement USceneTemplate as UDataAsset defining a scene template's configuration: scene ID (FString), display name, level asset reference (TSoftObjectPtr<UWorld>), available props (TArray<FSceneProp> with ID, display name, default active bool, transform), spawn points (TArray<FSceneSpawnPoint> with ID, label, transform), lighting presets (TMap<FString, FLightingPreset>), ambient sound cue reference, default music track reference. Create data assets for all 7 scenes per GDD Section 4.
**Acceptance:**
- [ ] Data asset class works
- [ ] All 7 scene templates defined
- [ ] Props and spawn points defined per GDD

### Task 7.2: Implement SceneDirector
**Status:** NOT_STARTED | **Deps:** 7.1 | **Team:** scene
**Instructions:** USceneDirector manages level streaming. LoadScene(SceneTemplateID) → streams in the level, unloads previous. Uses ULevelStreamingDynamic. L_Persistent is always loaded and hosts the HUD/game logic. Only one scene level loaded at a time. Fires FOnSceneLoaded delegate when ready.
**Acceptance:**
- [ ] Can load a scene template by ID
- [ ] Previous scene unloads
- [ ] FOnSceneLoaded fires when level is fully loaded
- [ ] L_Persistent remains loaded throughout

### Task 7.3: Implement SceneDresser
**Status:** NOT_STARTED | **Deps:** 7.2 | **Team:** scene
**Instructions:** USceneDresser applies the DM's scene parameters after a scene loads. Activates/deactivates props by ID, sets lighting preset, spawns NPC/enemy actors at spawn points, applies time-of-day and weather settings. Reads from FDMSceneChange struct.
**Acceptance:**
- [ ] Props activate/deactivate by ID
- [ ] Lighting preset changes
- [ ] Actors spawn at correct spawn points
- [ ] Scene visually updates based on DM parameters

### Task 7.4: Implement SceneTransition
**Status:** NOT_STARTED | **Deps:** 7.2 | **Team:** scene
**Instructions:** USceneTransition handles visual transitions. Fade to black (configurable duration, default 1.5s) → optional narration text overlay during load → fade in when new scene is ready. Uses UE5 camera fade or a full-screen widget.
**Acceptance:**
- [ ] Fade out/in works smoothly
- [ ] Narration text shows during transition
- [ ] Transition covers the level streaming load time

### Task 7.5: Build Tavern Scene Environment
**Status:** NOT_STARTED | **Deps:** 7.1 | **Team:** environment
**Instructions:** Build L_SCN_Tavern using Fab/Quixel marketplace assets. Follow GDD Section 4 "SCN_Tavern" layout exactly: L-shaped room, bar counter, fireplace, 4 tables, stairs, front door. Place all props with correct IDs matching the SceneTemplate data asset. Set up warm golden lighting (fireplace + candles). Apply Nanite to all static meshes. Set up Lumen GI. Configure for all 4 scalability presets.
**Acceptance:**
- [ ] Tavern environment loads and looks good
- [ ] All props are tagged with correct IDs
- [ ] Lighting matches GDD description (warm, golden, cozy)
- [ ] Spawn points exist for all NPCs (Marta, Durgan, Kael, patrons)
- [ ] Renders correctly on High preset

---

## Milestone 8: Character & Animation
**Status:** NOT_STARTED | **Dependencies:** Milestone 7

### Task 8.1: Set Up Player Character Model
**Status:** NOT_STARTED | **Deps:** 1.2 | **Team:** character
**Instructions:** Source a fantasy character model from Fab marketplace (or Mixamo) suitable for a generic human adventurer. Set up skeletal mesh, create Animation Blueprint (ABP_Player) with basic state machine: Idle, Walk, Run. Implement AGCPlayerCharacter with the mesh, animation BP, and basic movement (driven by ActionSystem, not player input). Player character does NOT have WASD movement — all movement is commanded by the ActionDirector.
**Acceptance:**
- [ ] Character model in editor with working skeleton
- [ ] Animation BP plays idle by default
- [ ] Walk/run animations blend correctly
- [ ] Character can be spawned in a scene

### Task 8.2: Create Animation Montage Library
**Status:** NOT_STARTED | **Deps:** 8.1 | **Team:** character
**Instructions:** Create Animation Montages for every action in the vocabulary (see GDD Section 3 "Available Actions Vocabulary"). Source animations from Mixamo or Fab packs, retarget to character skeleton.

**Required montages:**
- AM_Idle, AM_Walk, AM_Run, AM_Kneel, AM_Sit, AM_StandUp
- AM_AttackMelee, AM_AttackRanged, AM_CastSpell, AM_Block, AM_Dodge, AM_TakeHit, AM_Die
- AM_OpenContainer, AM_PickUpItem, AM_UseItem, AM_InteractObject
- AM_TalkGesture, AM_Nod, AM_ShakeHead, AM_Point, AM_Laugh, AM_ScaredReaction, AM_AngryGesture
- AM_Sneak, AM_Climb, AM_Jump

Store in Content/Characters/Player/Montages/. Same montages can be shared with NPC/enemy characters if they use compatible skeletons.
**Acceptance:**
- [ ] All listed montages exist and play correctly
- [ ] Montages can be triggered by name at runtime
- [ ] No animation glitches or T-pose fallbacks

### Task 8.3: Implement ActionDirector
**Status:** NOT_STARTED | **Deps:** 8.2 | **Team:** action
**Instructions:** UActionDirector receives FDMAction arrays from DMBrainSubsystem and executes them in sequence. For each action: move actor to target position (nav mesh or direct lerp), play animation montage, wait for completion, proceed to next. Handles delays between actions. Fires FOnActionSequenceComplete when all actions finish.
**Acceptance:**
- [ ] Receives action arrays and executes in order
- [ ] Actors move to specified positions
- [ ] Animation montages play correctly
- [ ] Delays between actions work
- [ ] Completion delegate fires

### Task 8.4: Implement CinematicCamera
**Status:** NOT_STARTED | **Deps:** 8.3 | **Team:** action
**Instructions:** UCinematicCamera manages camera framing during gameplay. Camera modes: establishing (wide shot of scene), medium (follow player), closeup (frame speaking character), reaction (cut to reacting character), combat (wide tactical). Camera auto-selects mode based on current action type. Uses interpolation for smooth transitions. Can also trigger pre-built Sequencer clips for special moments.
**Acceptance:**
- [ ] Camera switches between modes based on action type
- [ ] Smooth interpolation between camera positions
- [ ] Dialogue scenes cut between speaker and listener
- [ ] Combat scenes use wider framing

---

## Milestone 9: Action Translation (THE MAGIC MOMENT)
**Status:** NOT_STARTED | **Dependencies:** Milestones 6, 8

### Task 9.1: Wire DM Brain Actions to 3D World
**Status:** NOT_STARTED | **Deps:** 6.1, 8.3, 8.4, 7.2 | **Team:** integration
**Instructions:** Complete the full pipeline: player types → DM responds with JSON → actions parsed → ActionDirector executes in 3D → camera frames the action → narration displays.

Integrate:
1. DMBrainSubsystem.FOnDMActionsReady → ActionDirector.ExecuteActions()
2. DMBrainSubsystem.FOnDMSceneChangeRequested → SceneDirector.LoadScene()
3. ActionDirector uses CinematicCamera to frame each action
4. SceneDresser applies DM scene parameters after load

Test in the Tavern scene: player types "I sit at the bar and order an ale" → character walks to bar, sits, DM narrates Marta's response.

**THIS IS THE PROOF OF CONCEPT. When the player types a command and their character physically performs the action in 3D, the core vision is proven.**

**Acceptance:**
- [ ] Player types "I walk to the fireplace" → character walks to fireplace
- [ ] Player types "I sit at the table" → character sits
- [ ] Player types "I talk to the innkeeper" → character moves to bar, talk gesture plays, DM narrates Marta's dialogue
- [ ] Camera frames the action appropriately
- [ ] Narration text appears in sync with actions
- [ ] No crash on unknown actions (fallback to narration-only)

---

## Milestone 10: Dice Roll Visuals
**Status:** NOT_STARTED | **Dependencies:** Milestone 4, 5

### Task 10.1: Create 3D Dice Models
**Status:** NOT_STARTED | **Deps:** None | **Team:** art
**Instructions:** Create or source 3D models for d20, d12, d10, d8, d6, d4. Each die needs proper face numbering via textures or geometry. Set up physics materials so dice can roll convincingly. Store in Content/Dice/.
**Acceptance:**
- [ ] All 6 die types modeled with correct numbering
- [ ] Physics simulation rolls convincingly

### Task 10.2: Implement DiceRollDisplay
**Status:** NOT_STARTED | **Deps:** 10.1, 4.1 | **Team:** ui
**Instructions:** UDiceRollDisplay widget shows dice rolls on screen. When a roll occurs: spawn 3D die in a UI viewport or overlay, apply physics impulse, animate landing on the predetermined result. Below the die, show text: "[Check Type]: [roll] + [modifier] = [total] vs DC [target] — [Success/Failure!]". Display persists 4 seconds, then fades. Bind to DiceRoller's roll delegate.
**Acceptance:**
- [ ] Die appears and rolls when a check occurs
- [ ] Die lands showing the correct number
- [ ] Text overlay shows correct math
- [ ] Fades after 4 seconds
- [ ] Works for all die types

---

## Milestone 11: Cinematic Camera Polish
**Status:** NOT_STARTED | **Dependencies:** Milestone 8

### Task 11.1: Create Sequencer Camera Presets
**Status:** NOT_STARTED | **Deps:** 8.4 | **Team:** camera
**Instructions:** Create reusable Sequencer clips for common camera scenarios: dramatic_reveal (slow push-in on a character), combat_start (pull-out to wide), dialogue_over_shoulder (alternating shots), exploration_pan (slow sweep of environment), boss_entrance (low angle looking up). CinematicCamera selects and triggers these based on the action context.
**Acceptance:**
- [ ] At least 5 camera presets exist
- [ ] CinematicCamera can trigger them by name
- [ ] Smooth blending between presets

---

## Milestone 12: Companion (Kael)
**Status:** NOT_STARTED | **Dependencies:** Milestone 8, 3

### Task 12.1: Set Up Kael Character
**Status:** NOT_STARTED | **Deps:** 8.1 | **Team:** character
**Instructions:** Source/create Kael's character model (late 20s human male, dark hair, scar on jaw, leather-and-mail armor — see GDD Section 4). Set up skeletal mesh, Animation Blueprint (can share base ABP with player), create AGCCompanionCharacter. Kael uses the same animation montage library as the player (shared skeleton).
**Acceptance:**
- [ ] Kael model loads in editor
- [ ] Animations work correctly
- [ ] Can be spawned in a scene at a spawn point

### Task 12.2: Implement Companion AI Combat Decisions
**Status:** NOT_STARTED | **Deps:** 12.1, 2.1, 3.2 | **Team:** ai-core
**Instructions:** During combat, Kael's actions are decided by llama3.1:8b using the Companion Combat Prompt (see GDD Section 3). Send Kael's sheet, combat situation, and available actions to the model. Parse the JSON response (action, target, comment). Feed the action into ActionDirector. Display Kael's quip in a speech bubble or in the DM text panel as "Kael: [comment]".
**Acceptance:**
- [ ] Kael's combat action request goes to llama3.1:8b
- [ ] Response parses correctly
- [ ] Kael performs the action in 3D
- [ ] Combat quips display
- [ ] Latency < 1s

### Task 12.3: Implement Companion Exploration Behavior
**Status:** NOT_STARTED | **Deps:** 12.1, 3.6 | **Team:** ai-core
**Instructions:** Outside combat, Kael follows the player, reacts to DM narration, and occasionally comments. The DM's response includes "companion_reaction" field — display this in the text panel as "Kael: [reaction]". Kael should idle near the player, face whatever the player is interacting with, and play occasional ambient animations (looking around, crossing arms, etc.).
**Acceptance:**
- [ ] Kael follows player between scene positions
- [ ] Companion reactions display from DM responses
- [ ] Ambient idle behavior works

---

## Milestone 13: NPC Dialogue
**Status:** NOT_STARTED | **Dependencies:** Milestone 8, 3

### Task 13.1: Create NPC Character Models
**Status:** NOT_STARTED | **Deps:** 8.1 | **Team:** character
**Instructions:** Source/create models for Marta (middle-aged woman, apron), Old Durgan (ancient dwarf, white beard), Village Merchant (cheerful halfling, colorful vest). Implement AGCNPCCharacter with basic idle animations and talk gestures. NPCs stay at their designated positions in the scene.
**Acceptance:**
- [ ] All 3 NPC models load and animate
- [ ] NPCs can be spawned at scene positions
- [ ] Talk gesture animation plays during dialogue

### Task 13.2: Wire NPC Dialogue Through DM Brain
**Status:** NOT_STARTED | **Deps:** 13.1, 3.6 | **Team:** integration
**Instructions:** When the player interacts with an NPC, the DM brain handles the dialogue (the DM "voices" all NPCs). The DM prompt includes the NPC's personality block (loaded from text asset). Camera cuts to dialogue mode (close-ups alternating between player and NPC). NPC plays talk gestures while their dialogue is shown in the text panel.

Test in Tavern: player approaches Marta, types "I ask about the disappearances" → camera cuts to Marta, DM narrates her response in character, Marta plays talk animation.
**Acceptance:**
- [ ] NPC dialogue flows naturally through the DM
- [ ] Camera frames dialogue correctly
- [ ] NPC personality is consistent (Marta sounds like Marta)
- [ ] Multiple exchanges work (follow-up questions)

---

## Milestone 14: Combat System
**Status:** NOT_STARTED | **Dependencies:** Milestones 4, 9, 12

### Task 14.1: Implement Combat Flow Manager
**Status:** NOT_STARTED | **Deps:** 4.3, 9.1, 12.2 | **Team:** gameplay
**Instructions:** When the DM triggers combat (via world_change "start_combat"), enter combat mode:
1. DM narrates combat beginning
2. Rules engine rolls initiative for all combatants
3. Sort by initiative, display turn order
4. On player's turn: enable input, player types their action
5. On companion's turn: llama3.1:8b decides, action executes
6. On enemy's turn: DM decides enemy actions, rules engine resolves
7. Repeat until combat ends (all enemies dead, fled, or DM ends it)

Combat UI additions: subtle turn-order indicator, "In Combat" border/indicator.
**Acceptance:**
- [ ] Combat mode initiates from DM trigger
- [ ] Initiative rolls and sorts correctly
- [ ] Player can act on their turn
- [ ] Companion acts autonomously
- [ ] Enemies act (DM decides, rules resolve)
- [ ] Combat ends when conditions met

### Task 14.2: Implement Enemy Characters
**Status:** NOT_STARTED | **Deps:** 8.1 | **Team:** character
**Instructions:** Source/create models for Goblin, Skeleton, Skeleton Warrior. Implement AGCEnemyCharacter. Enemies use the same animation vocabulary as other characters (attack, take_hit, die). Create enemy stat block data assets per GDD Section 4 enemy stat blocks.
**Acceptance:**
- [ ] All 3 enemy types have models and animations
- [ ] Stat block data assets match GDD
- [ ] Enemies can be spawned at scene positions

### Task 14.3: Combat End-to-End Test
**Status:** NOT_STARTED | **Deps:** 14.1, 14.2 | **Team:** integration
**Instructions:** Set up the goblin ambush encounter in the forest scene. Player + Kael vs 2-3 goblins. Full combat loop: initiative → turns → attacks → damage → death. Verify: dice rolls display, HP updates, death animations play, DM narrates each action, combat ends when goblins are defeated.
**Acceptance:**
- [ ] Full combat loop plays through without crashes
- [ ] Dice rolls visible for every attack/check
- [ ] HP bars update correctly
- [ ] Enemies play death animation and are removed
- [ ] DM narrates throughout
- [ ] Combat ends and returns to exploration mode

---

## Milestone 15: Remaining Scenes
**Status:** NOT_STARTED | **Dependencies:** Milestone 7

### Task 15.1: Build Village Square Scene
**Status:** NOT_STARTED | **Deps:** 7.1 | **Team:** environment
**Instructions:** Build L_SCN_VillageSquare per GDD. Open area, well, market stalls, building facades, road south. Natural daylight lighting. Spawn points for merchant and villagers.
**Acceptance:** Scene loads, looks good, props tagged, lighting matches GDD.

### Task 15.2: Build Forest Path Scene
**Status:** NOT_STARTED | **Deps:** 7.1 | **Team:** environment
**Instructions:** Build L_SCN_ForestPath per GDD. Linear path, dense trees, clearing for ambush, fog volume, god rays. Increasingly ominous lighting.
**Acceptance:** Scene loads, atmosphere is tense, clearing suitable for combat.

### Task 15.3: Build Cavern Entrance Scene
**Status:** NOT_STARTED | **Deps:** 7.1 | **Team:** environment
**Instructions:** Build L_SCN_CavernEntrance per GDD. Rocky hillside, dark cave mouth, mining debris. High contrast between grey daylight and cave darkness.
**Acceptance:** Scene loads, cave mouth is foreboding, props placed.

### Task 15.4: Build Dungeon Corridor Scene
**Status:** NOT_STARTED | **Deps:** 7.1 | **Team:** environment
**Instructions:** Build L_SCN_DungeonCorridor per GDD. Narrow tunnel, timber supports, torch sconces, puddles, cobwebs. Near-darkness with torch pools.
**Acceptance:** Scene loads, claustrophobic atmosphere, torches provide dramatic lighting.

### Task 15.5: Build Dungeon Chamber Small Scene
**Status:** NOT_STARTED | **Deps:** 7.1 | **Team:** environment
**Instructions:** Build L_SCN_DungeonChamberSmall per GDD. Reusable room with toggleable props (chest, altar, cells, crates, ritual circle). All props default inactive, activated by SceneDresser.
**Acceptance:** Scene loads, props toggle on/off correctly, multiple configurations work.

### Task 15.6: Build Boss Chamber Scene
**Status:** NOT_STARTED | **Deps:** 7.1 | **Team:** environment
**Instructions:** Build L_SCN_DungeonChamberBoss per GDD. Large cavern, raised platform, bone throne, cursed artifact with emissive glow, pillars for cover, skeleton piles. Dramatic green/purple artifact lighting.
**Acceptance:** Scene loads, boss area is dramatic, artifact glows, lighting is climactic.

---

## Milestone 16: Enemy AI
**Status:** NOT_STARTED | **Dependencies:** Milestones 14, 15

### Task 16.1: Implement Hollow King Boss
**Status:** NOT_STARTED | **Deps:** 14.2 | **Team:** character
**Instructions:** Source/create Hollow King model (gaunt undead scholar, grey skin, glowing green eyes, tattered robes — see GDD Section 4). Create stat block data asset. Implement boss-specific behavior: dialogue before combat (DM-driven), lair action (50% chance to summon skeleton each round), weakness (artifact can be destroyed). The Hollow King is fully DM-controlled — the DM decides his actions in combat and his dialogue responses.
**Acceptance:**
- [ ] Model looks appropriately menacing
- [ ] Stat block matches GDD
- [ ] Can participate in dialogue and combat
- [ ] Lair actions work

---

## Milestone 17: The Adventure (FULL MVP)
**Status:** NOT_STARTED | **Dependencies:** Milestones 13-16, 18

### Task 17.1: Script Adventure Flow
**Status:** NOT_STARTED | **Deps:** All prior milestones | **Team:** design
**Instructions:** Create the adventure outline that gets injected into the DM's prompt. This is NOT a rigid script — it's guidance for the DM on the story structure and available events. See GDD Section 4 "Narrative Flow" for the three-act structure.

Create a text asset with:
- Act 1 guidance (tavern: quest pickup, NPC meetings, companion recruitment)
- Act 2 guidance (forest: ambush encounter, atmosphere building)
- Act 3 guidance (dungeon: exploration, encounters, boss fight, multiple resolutions)
- Key plot points that must happen for the story to progress
- Scene transitions the DM should trigger at appropriate moments
- The Hollow King's dialogue options and resolution paths

**Acceptance:**
- [ ] Adventure outline text asset exists
- [ ] Covers all three acts
- [ ] Includes all key plot points
- [ ] DM prompt builder includes this in the prompt

### Task 17.2: Full Adventure Playtest
**Status:** NOT_STARTED | **Deps:** 17.1 | **Team:** integration
**Instructions:** Play through the entire adventure start to finish. Test:
- Character selection → tavern arrival
- All NPC conversations (Marta, Durgan, Merchant, Kael)
- Scene transitions between all 7 scenes
- Goblin ambush combat
- Dungeon exploration (traps, skeleton encounters)
- Hollow King encounter (test at least 2 resolution paths)
- Adventure completion and epilogue

Document bugs, prompt tuning needs, pacing issues, and animation gaps.

**THIS IS THE MVP PLAYTEST. The game should be playable for 30-60 minutes without crashes.**

**Acceptance:**
- [ ] Full adventure completes without crashes
- [ ] All scene transitions work
- [ ] All NPC dialogues are in-character and responsive
- [ ] Both combat encounters play correctly
- [ ] Hollow King encounter allows multiple resolution approaches
- [ ] Save/load works mid-adventure
- [ ] 30-60 minute runtime is achieved

---

## Milestone 18: Inventory & Items
**Status:** NOT_STARTED | **Dependencies:** Milestone 4, 5

### Task 18.1: Implement Inventory System
**Status:** NOT_STARTED | **Deps:** 4.4, 5.3 | **Team:** gameplay
**Instructions:** UInventoryComponent on player and companion. ItemDefinition data assets for all MVP items (see GDD Section 4 "Items & Inventory"). Inventory UI panel accessible from character sheet. Items added via DM world_changes (add_to_inventory). Healing potion use via player command ("I use a healing potion"). Gold tracking.
**Acceptance:**
- [ ] Items can be added to and removed from inventory
- [ ] All MVP items have data assets
- [ ] Inventory UI shows current items
- [ ] Healing potion heals correctly
- [ ] Gold tracked

### Task 18.2: Implement Merchant Interaction
**Status:** NOT_STARTED | **Deps:** 18.1, 13.2 | **Team:** gameplay
**Instructions:** When player talks to Village Merchant, DM handles the shop interaction. Player says "I want to buy a healing potion" → DM checks gold, processes transaction via world_changes. DM can also handle haggling via Persuasion checks.
**Acceptance:**
- [ ] Can buy items from merchant
- [ ] Gold deducted correctly
- [ ] DM handles haggling naturally

---

## Milestone 19: Memory & Save
**Status:** NOT_STARTED | **Dependencies:** Milestone 3

### Task 19.1: Implement DMMemoryManager with SQLite
**Status:** NOT_STARTED | **Deps:** 3.5 | **Team:** ai-core
**Instructions:** UDMMemoryManager manages Layer 3 (long-term) memory. Uses SQLite (UE5 SQLiteCore module) to store: full conversation logs, NPC interaction summaries, memory summaries. After every 5 exchanges, send recent history to llama3.1:8b for summarization, store summary in SQLite. When an NPC is encountered, pull their interaction history from SQLite into the active context.
**Acceptance:**
- [ ] SQLite database creates and stores data
- [ ] Conversation logs persist
- [ ] NPC histories retrievable
- [ ] Summarization runs in background via llama3.1:8b

### Task 19.2: Implement Save/Load System
**Status:** NOT_STARTED | **Deps:** 19.1, 3.6 | **Team:** gameplay
**Instructions:** UGCSaveSubsystem and UGCSaveGame. Save: player sheet, companion sheet, adventure state, current scene, recent conversation history. SQLite database file saved alongside. Load: restore all state, load correct scene, rebuild DM context. Save triggers: F5 hotkey, "save" typed as player input. DM acknowledges save in-fiction.
**Acceptance:**
- [ ] F5 saves the game
- [ ] "save" typed as input triggers save + DM acknowledgment
- [ ] Load from main menu restores full game state
- [ ] Player can save mid-adventure and resume correctly
- [ ] SQLite data persists across save/load

---

## Milestone 20: Audio
**Status:** NOT_STARTED | **Dependencies:** Milestone 15

### Task 20.1: Source and Implement Music
**Status:** NOT_STARTED | **Deps:** 15.1-15.6 | **Team:** audio
**Instructions:** Source royalty-free orchestral fantasy music tracks from Fab or free libraries. Needed: tavern_warm, village_calm, forest_tense, dungeon_dread, combat_intense, boss_epic, victory, death. Implement dynamic music system: SceneDirector triggers track changes on scene load, CombatFlowManager triggers combat music. Crossfade transitions (2-3 second fade).
**Acceptance:**
- [ ] All 8 music tracks sourced and imported
- [ ] Music changes with scenes
- [ ] Combat music triggers on combat start
- [ ] Crossfade transitions smooth

### Task 20.2: Implement Ambient Sound and SFX
**Status:** NOT_STARTED | **Deps:** 15.1-15.6 | **Team:** audio
**Instructions:** Per-scene ambient sound beds (see GDD Section 5 "Ambient Sound"). SFX for: sword impacts, spell sounds, door/chest interactions, footsteps (surface-specific), dice clatter, UI sounds. Source from Fab or free SFX libraries.
**Acceptance:**
- [ ] Each scene has appropriate ambient audio
- [ ] Combat SFX play with attacks
- [ ] Interaction SFX play with actions
- [ ] Dice roll has physical clatter sound
- [ ] UI sounds for text input and notifications

---

## Milestone 21: Settings & Scalability
**Status:** NOT_STARTED | **Dependencies:** Milestone 1

### Task 21.1: Configure Scalability Presets
**Status:** NOT_STARTED | **Deps:** 1.5 | **Team:** infrastructure
**Instructions:** Configure Scalability.ini with 4 presets per GDD Section 5 "Scalable Graphics Pipeline." Set up: GI (SSGI→Lumen→RT Lumen), shadows (CSM→VSM→RT), geometry (LOD→Nanite), reflections (SSR→Lumen→RT), post processing levels. Enable DLSS/FSR/XeSS plugin and expose in settings. Auto-detect GPU and recommend preset.
**Acceptance:**
- [ ] All 4 presets apply correctly
- [ ] Measurable performance difference between presets
- [ ] DLSS/FSR/XeSS toggle works
- [ ] Auto-detection recommends appropriate preset

---

## Milestone 22: Polish & Playtesting
**Status:** NOT_STARTED | **Dependencies:** All prior milestones

### Task 22.1: Prompt Tuning
**Status:** NOT_STARTED | **Deps:** 17.2 | **Team:** ai-core
**Instructions:** Based on playtest results from M17, tune all DM prompts. Focus on: consistent JSON output, in-character NPC voices, appropriate difficulty of checks/DCs, pacing of narration, quality of combat narration, Hollow King dialogue quality, Kael's personality consistency. Iterate on system prompts until the DM feels natural and engaging.
**Acceptance:**
- [ ] DM consistently produces valid JSON (>95% success rate)
- [ ] NPC voices are distinct and in-character
- [ ] Combat narration is exciting
- [ ] Hollow King encounter is dramatic
- [ ] Kael feels like a real companion

### Task 22.2: Animation and Camera Polish
**Status:** NOT_STARTED | **Deps:** 17.2 | **Team:** action
**Instructions:** Fix animation timing issues, add blending between montages, polish camera transitions. Ensure action sequences look cinematic — no teleporting, no awkward pauses, no camera snaps.
**Acceptance:**
- [ ] All action sequences play smoothly
- [ ] Camera transitions are cinematic
- [ ] No visual glitches during actions

### Task 22.3: UI Polish
**Status:** NOT_STARTED | **Deps:** 17.2 | **Team:** ui
**Instructions:** Polish all UI elements: consistent fonts, proper scaling at different resolutions, hover/click states on buttons, loading indicators, error messages. Ensure the DM text panel is readable against all scene backgrounds.
**Acceptance:**
- [ ] UI is consistent and polished
- [ ] Readable at 1080p, 1440p, and 4K
- [ ] All interactive elements have proper states

### Task 22.4: Bug Fix Pass
**Status:** NOT_STARTED | **Deps:** 17.2 | **Team:** all
**Instructions:** Fix all bugs documented during M17 playtest. Focus on: crash bugs first, then game-breaking bugs, then visual bugs, then minor polish.
**Acceptance:**
- [ ] No crash bugs remain
- [ ] No game-breaking bugs remain
- [ ] Visual bugs are minimal

### Task 22.5: Final Playthrough
**Status:** NOT_STARTED | **Deps:** 22.1-22.4 | **Team:** integration
**Instructions:** Complete a final start-to-finish playthrough of the adventure. Verify the full experience is fun, polished, and stable.
**Acceptance:**
- [ ] Full adventure completes without issues
- [ ] The experience is fun and engaging
- [ ] The AI DM feels like a real Dungeon Master
- [ ] Graphics look good on the user's RTX 5080
- [ ] Save/load works correctly
- [ ] Settings menu works
- [ ] Total playtime is 30-60 minutes

---

## Dependency Graph (Quick Reference)

```
M1 (Scaffolding) ──→ M2 (Ollama) ──→ M3 (DM Brain) ──→ M6 (Talking Demo)
      │                                      │                    │
      │                                      ↓                    │
      │                               M4 (Rules Engine) ─────────→│
      │                                      │                    │
      │                                      ↓                    ↓
      ├──→ M7 (Scenes) ──→ M8 (Characters) ──→ M9 (ACTION TRANSLATION)
      │         │                │                    │
      │         ↓                ↓                    ↓
      │    M15 (All Scenes)  M12 (Kael)         M10 (Dice Visuals)
      │         │            M13 (NPC Dialogue)  M11 (Camera Polish)
      │         │                │
      │         ↓                ↓
      │    M16 (Enemy AI)   M14 (Combat) ──→ M17 (THE ADVENTURE / MVP)
      │                                            │
      ├──→ M18 (Inventory) ────────────────────────→│
      ├──→ M19 (Memory/Save) ─────────────────────→│
      ├──→ M20 (Audio) ───────────────────────────→│
      ├──→ M21 (Scalability) ─────────────────────→│
      │                                            │
      │                                            ↓
      └─────────────────────────────────────→ M22 (POLISH)
```

**Critical Path:** M1 → M2 → M3 → M6 → M9 → M14 → M17 → M22
**Key Milestones:** M6 (first playable text), M9 (3D proof of concept), M17 (full MVP)
