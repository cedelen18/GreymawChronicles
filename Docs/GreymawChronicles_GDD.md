# Greymaw Chronicles — Game Design Document
## AI Dungeon Master D&D Experience

**Version:** 1.0
**Engine:** Unreal Engine 5.7.3
**Platform:** Windows (PC)
**Target Hardware:** RTX 5080, 64GB RAM, Intel Core Ultra 7 265KF
**Local LLM:** Ollama (localhost:11434)
**Scope:** MVP — Single self-contained adventure (30-60 minutes)

---

## Table of Contents

1. [Core Concept](#1-core-concept)
2. [Player Experience](#2-player-experience)
3. [AI Brain Design](#3-ai-brain-design)
4. [World & Content](#4-world--content)
5. [Art & Audio Direction](#5-art--audio-direction)
6. [Technical Architecture](#6-technical-architecture)
7. [Scope Control](#7-scope-control)

---

## 1. Core Concept

### Elevator Pitch

A living D&D session where an AI Dungeon Master runs a 5th Edition adventure and a real-time 3D world serves as the "theater of the mind." Instead of imagining what happens, you watch your character perform every action cinematically in Unreal Engine 5. The AI DM narrates, controls NPCs, resolves dice rolls using real 5e rules, and adapts the story to your choices.

### Core Gameplay Loop

1. **AI DM sets a scene** → the 3D world loads/transitions to that environment
2. **Player sees** their character and any NPCs placed in the scene
3. **AI DM narrates** the situation via text panel
4. **Player types** what they want to do in natural language
5. **AI DM interprets** the action, determines if a roll is needed
6. **5e rules engine** resolves dice rolls deterministically (shown on screen)
7. **Characters physically perform** the action cinematically in the 3D world
8. **World state updates**, DM remembers everything, story continues

### Hook

"What if your D&D Dungeon Master could show you the adventure instead of just telling you?"

The AI DM is a translator between natural language intent and cinematic game actions, plus a storyteller, referee, and world-builder. The player has the freedom of tabletop D&D with the visual spectacle of a AAA RPG.

### Tone & Aesthetic

Classic high fantasy D&D — torchlit dungeons, medieval villages, dramatic spell effects. Cinematic quality scenes driven entirely by AI narration and player choice.

### Reference Games

- **Baldur's Gate 3** — 5e mechanics, high-fantasy tone, companion system
- **AI Dungeon** — AI-driven freeform narrative
- **Demeo** — scene-based D&D visualization
- **Solasta: Crown of the Magister** — faithful 5e rules implementation

### Scope

- **MVP:** One self-contained adventure (tavern → forest → dungeon → boss). 30-60 min playtime. One player character, 1 AI companion, handful of enemy types, 7 scene templates.
- **Full Game Vision:** A campaign framework where the AI DM runs extended multi-session adventures with persistent world state, character progression, multiple companions, procedural quest generation, and emergent storylines.

---

## 2. Player Experience

### Character Creation (MVP)

Menu-driven selection from pre-built Level 3 characters:

**Races (3):**
- Human — +1 to all ability scores. Versatile.
- Elf — +2 Dexterity. Darkvision, Perception proficiency.
- Dwarf — +2 Constitution. Darkvision, Poison resistance.

**Classes (4):**
- **Fighter** — Heavy armor, martial weapons, Second Wind, Action Surge. Subclass: Champion (improved critical).
  - Starting HP: 28 (10 + 6 + 6 + 6 CON mod assumed +2 each level)
  - AC: 18 (chain mail + shield)
  - Primary weapon: Longsword (1d8+3)
- **Rogue** — Light armor, finesse weapons, Sneak Attack (2d6), Cunning Action. Subclass: Thief.
  - Starting HP: 21
  - AC: 14 (leather armor + DEX)
  - Primary weapon: Shortsword (1d6+3) or Shortbow (1d6+3)
- **Wizard** — Arcane spellcasting, spell slots (4/2 at level 3), prepared spells. Subclass: Evocation.
  - Starting HP: 14
  - AC: 12 (Mage Armor spell)
  - Cantrips: Fire Bolt (2d10), Mage Hand, Prestidigitation
  - Key spells: Magic Missile, Shield, Thunderwave, Scorching Ray, Misty Step
- **Cleric** — Divine spellcasting, medium armor, healing. Subclass: Life Domain.
  - Starting HP: 24
  - AC: 16 (scale mail + shield)
  - Cantrips: Sacred Flame, Guidance, Spare the Dying
  - Key spells: Cure Wounds, Healing Word, Guiding Bolt, Spiritual Weapon, Aid

Each pre-built character has a complete 5e stat block stored as a `UDataAsset` including: ability scores, saving throw proficiencies, skill proficiencies, equipment, class features, spell slots (if applicable), and spell list.

**Player customization:** Name input, appearance preset selection (3-4 visual presets per race/class combo).

### Character Creation (Full Game Vision)

AI-driven conversational character creation: the DM walks you through race, class, ability scores, background, and backstory selection through natural language dialogue.

### Camera System

**Scene-based cinematic camera.** The DM sets scenes and the camera system frames the action:

- **Establishing shot** when a new scene loads — wide angle showing the environment
- **Medium shot** during exploration — follows the player character at a comfortable distance
- **Close-up** during dialogue — cuts to the speaking character's face
- **Reaction shots** — cuts to other characters reacting during important moments
- **Combat framing** — pulls to a wider tactical view, zooms in for attacks and spell effects
- **Dynamic cuts** — the CinematicCamera system has pre-built Sequencer clips for common framings that it selects based on the current action

The camera is NOT player-controlled. The player does not move their character with WASD. All movement and positioning is driven by the AI DM's stage directions. The player's only input is the text box.

### Input Method

**Text input box** at the bottom of the screen. Chat-style interface overlaid on the 3D world. The player types natural language commands to the DM and presses Enter to send.

Examples of valid player inputs:
- "I walk over to the bar and order an ale"
- "I search the chest for traps before opening it"
- "I want to sneak past the guards"
- "I cast Fireball at the group of goblins"
- "I try to convince the Hollow King to surrender"
- "What does Kael think about this?"
- "I examine the runes on the wall more closely"

### Dice Roll Display

When any check, attack, or save occurs:
1. A 3D die (d20, d12, d10, d8, d6, d4 — whichever is appropriate) rolls across the screen with physics-based animation
2. The die lands on the actual result number
3. A UI overlay shows: `[Roll Type]: [Die Result] + [Modifier] = [Total] vs DC [Target] — [Success/Failure]`
4. Example: `Persuasion Check: 13 + 4 = 17 vs DC 15 — Success!`
5. The display persists for 3-4 seconds, then fades

The roll is determined by the C++ rules engine FIRST, then the dice animation is synced to show that result. The animation is cosmetic — the math is already done.

### Party System (MVP)

- **Player:** 1 character, the protagonist
- **Companion:** Kael, Human Fighter. AI-controlled. The DM controls Kael's actions, dialogue, and combat decisions. Kael has his own personality, opinions, and reactions.

### Party System (Full Game Vision)

- Full party of up to 4 characters (1 player + 3 AI companions)
- Multiple companion options: Kael (Fighter), Lyra (Elf Cleric), and more
- Companion relationship system — companions react to player choices, can approve or disapprove, have personal quests
- Companions can leave the party if relationship drops too low

### Win/Loss Conditions

- **Win:** Complete the adventure objective (defeat or resolve the Hollow King encounter)
- **Loss:** Player character reaches 0 HP and fails death saving throws (standard 5e death mechanics). Companion can attempt to stabilize you.
- **On death:** The DM narrates the death, offers to reload the last save. No permadeath for MVP.

### Session Structure

A single continuous play session of 30-60 minutes. The adventure has a clear beginning (tavern), middle (dungeon exploration), and end (boss encounter). The player can save at any time and resume later.

---

## 3. AI Brain Design

### Architecture Overview

The AI DM is a **hybrid system** combining LLM intelligence with deterministic C++ game logic:

```
Player Input (text)
       ↓
[AI DM Brain - qwen2.5:32b]
  - Interprets player intent
  - Decides narrative outcome
  - Determines if dice roll is needed
  - Outputs structured JSON + narration
       ↓
[5e Rules Engine - C++ Code]        [Action Parser - C++ Code]
  - Rolls dice deterministically     - Validates JSON actions
  - Applies 5e math                  - Maps to animation vocabulary
  - Calculates HP/damage/etc         - Queues scene changes
       ↓                                    ↓
[AI DM Brain - Second Pass]          [Unreal Engine Execution]
  - Receives roll results             - Plays animations
  - Narrates the outcome              - Moves actors
  - Updates story state               - Loads scenes
       ↓                                    ↓
[Player sees: Dice roll on screen + Character performs action + DM narration text]
```

### Two-Pass Resolution

For actions that require dice rolls, the DM is called twice:

**Pass 1 (Player Action → DM Decision):**
- Input: player's text + full context
- Output: what check/roll is needed, DC, AND pre-written success/failure narrations + actions
- The DM provides BOTH outcomes so we only need one LLM call for most situations

**Pass 2 (Only if needed):**
- For complex outcomes where the DM needs to react to the specific roll result
- Example: critical hits, critical failures, or rolls that are very close to the DC
- Input: the roll result + context
- Output: adjusted narration and actions

Most interactions resolve in a single LLM call (Pass 1 provides both branches). Pass 2 is the exception, not the rule.

### Model Assignment

| Game System | Model | Latency Target | Notes |
|---|---|---|---|
| DM Narration & Storytelling | qwen2.5:32b | <3s | Primary DM brain. Creative writing + structured output. |
| NPC Dialogue & Personality | qwen2.5:32b | <2s | Same model, different system prompt per NPC personality. |
| Scene Direction (JSON) | qwen2.5:32b | <3s | Best-in-class structured output for action/scene JSON. |
| 5e Rules Decisions | qwen2.5:32b | <2s | Decides what check is needed. C++ does the math. |
| Companion Combat AI | llama3.1:8b | <500ms | Simpler task — pick action from constrained list. Fast. |
| Quest Generation | qwen2.5:32b | <5s | Runs during scene transitions. Latency relaxed. |
| Memory Summarization | llama3.1:8b | Background | Periodically summarize recent events for long-term storage. |

### Memory System

**Layered memory architecture:**

**Layer 1 — Active Context (always in prompt, ~2000-3000 tokens):**
- Current scene description and props
- Active NPCs present (names, dispositions)
- Player character sheet (compact JSON)
- Companion character sheet (compact JSON)
- Current quest objective and progress
- Last 10-15 conversation exchanges (player input + DM response summaries)

**Layer 2 — Adventure State (always in prompt, ~500 tokens):**
- Key plot flags as JSON: `{"tavern_quest_accepted": true, "gate_passed": true, "goblins_defeated": true, ...}`
- NPC relationship states: `{"marta": "grateful", "kael": "friendly", ...}`
- Inventory state

**Layer 3 — Long-term Memory (SQLite database, retrieved as needed):**
- Full conversation logs (every exchange)
- Detailed NPC interaction history (loaded when that NPC is present)
- World lore details
- Retrieved by relevance using keyword matching when context requires it

**Memory Management:**
- After every 5 exchanges, llama3.1:8b summarizes the recent exchanges and stores them in Layer 3
- When an NPC is encountered again, their full interaction history is pulled from Layer 3 into Layer 1
- Adventure State (Layer 2) is updated in real-time as flags change

### Prompt Architecture

**System Prompt Structure (~3000-4000 tokens total):**

```
SECTION 1: DM PERSONALITY AND RULES (always present)
──────────────────────────────────────────────────────
You are the Dungeon Master for a 5th Edition Dungeons & Dragons adventure called
"The Greymaw Caverns." Your style is vivid, dramatic, and immersive. You describe
scenes with sensory detail — what the player sees, hears, smells. You are fair but
challenging. You never break character or acknowledge being an AI.

When the player describes an action:
1. Determine if a dice roll is required
2. If yes: specify the check type, ability, DC, and provide BOTH success and failure
   outcomes including narration and stage directions
3. If no roll needed: provide the narration and stage directions directly

You must ALWAYS respond in the specified JSON format. Never respond with plain text.
You may only use animations and actions from the AVAILABLE ACTIONS list.
If a player's desired action has no matching animation, narrate it descriptively
but do not include an action entry — the camera will hold on the scene while the
narration plays.

SECTION 2: AVAILABLE ACTIONS VOCABULARY (always present)
──────────────────────────────────────────────────────────
ANIMATIONS:
- idle, walk, run, kneel, sit, stand_up
- attack_melee, attack_ranged, cast_spell, block, dodge, take_hit, die
- open_container, pick_up_item, use_item, interact_object
- talk_gesture, nod, shake_head, point, laugh, scared_reaction, angry_gesture
- sneak, climb, jump

ACTOR TARGETS:
- "player", "companion_kael", "npc_[name]", "enemy_[id]"

PROPS:
- Defined per scene template. The DM is told which props exist in the current scene.

SECTION 3: OUTPUT FORMAT SPECIFICATION (always present)
────────────────────────────────────────────────────────
You must respond with a JSON object matching this schema:

{
  "narration": "string — the DM's narration text shown to the player",
  "check_required": null | {
    "type": "string — e.g. perception, athletics, persuasion, attack, saving_throw",
    "ability": "string — strength, dexterity, constitution, intelligence, wisdom, charisma",
    "dc": number,
    "advantage": boolean,
    "disadvantage": boolean
  },
  "on_success": {
    "narration": "string — narration if the check succeeds",
    "actions": [
      {
        "actor": "string — actor identifier",
        "animation": "string — from ANIMATIONS list",
        "target": "string | null — target actor or prop",
        "move_to": "string | null — position/prop to move toward",
        "delay": number | null — seconds to wait before this action (default 0)
      }
    ],
    "world_changes": [
      {
        "type": "string — set_flag, add_to_inventory, remove_from_inventory,
                 damage, heal, set_prop_state, spawn_npc, remove_npc,
                 change_scene, start_combat, end_combat",
        ... type-specific fields ...
      }
    ]
  },
  "on_failure": { ... same structure as on_success ... } | null,
  "companion_reaction": "string | null — Kael's spoken reaction or comment",
  "scene_change": null | {
    "template": "string — scene template ID",
    "lighting": "string — bright, dim, dark, torchlit",
    "time_of_day": "string — day, dusk, night",
    "weather": "string — clear, overcast, rain, fog",
    "props": ["string — additional props to place"],
    "npcs_to_spawn": ["string — NPC IDs to place in scene"]
  }
}

If no check is required, set check_required to null, on_failure to null,
and put the narration and actions directly in on_success.

SECTION 4: CURRENT CONTEXT (updated every turn)
────────────────────────────────────────────────
Current Scene: [scene template ID and description]
Props in Scene: [list of interactable props with IDs]
NPCs Present: [list with names and current state]
Lighting: [current lighting state]

SECTION 5: CHARACTER SHEETS (updated when stats change)
───────────────────────────────────────────────────────
PLAYER CHARACTER:
{
  "name": "...", "race": "Human", "class": "Fighter", "level": 3,
  "hp": {"current": 28, "max": 28},
  "ac": 18,
  "abilities": {"str": 16, "dex": 14, "con": 14, "int": 10, "wis": 12, "cha": 10},
  "proficiencies": ["athletics", "perception", "intimidation"],
  "features": ["fighting_style_defense", "second_wind", "action_surge", "improved_critical"],
  "equipment": ["longsword", "shield", "chain_mail", "explorers_pack"],
  "inventory": ["healing_potion_x2", "gold_25"]
}

COMPANION — KAEL:
{
  "name": "Kael", "race": "Human", "class": "Fighter", "level": 3,
  "personality": "Stoic ex-soldier with dry humor. Loyal but cautious.
                  Prefers direct action over diplomacy. Has a soft spot for
                  underdogs. Addresses the player formally at first, warms up
                  over time.",
  "hp": {"current": 28, "max": 28},
  "ac": 18,
  ... (full stat block)
}

SECTION 6: ADVENTURE STATE (updated as flags change)
──────────────────────────────────────────────────────
{
  "current_quest": "Investigate the disappearances in Greymaw Caverns",
  "quest_stage": "tavern_arrival",
  "flags": {
    "talked_to_marta": false,
    "recruited_kael": false,
    "talked_to_durgan": false,
    "bought_supplies": false,
    "entered_forest": false,
    "goblin_encounter_resolved": false,
    "entered_caverns": false,
    "found_pendant": false,
    "reached_hollow_king": false,
    "adventure_complete": false
  },
  "npcs_met": [],
  "enemies_defeated": 0,
  "companion_relationship": "neutral"
}

SECTION 7: CONVERSATION HISTORY (rolling window)
─────────────────────────────────────────────────
[Last 10-15 exchanges in chronological order]
Player: "..."
DM: [summary of response]
Player: "..."
DM: [summary of response]
...
```

### NPC Personality Prompts

Each NPC has a dedicated personality block injected into the system prompt when they are present in the scene:

**Marta the Innkeeper:**
```
NPC — MARTA (Innkeeper of the Thornhaven Tavern):
Personality: Worried, maternal, practical. Her nephew Aldric is among the missing.
Speech: Warm but strained. Uses "dear" and "love" as terms of address.
         Speaks with a slight rural dialect.
Knowledge: Knows the village history, that disappearances started 3 weeks ago,
           that victims went toward the old caverns, that miners abandoned those
           caves years ago after finding "something wrong" deep inside.
Goal: Wants someone brave enough to investigate. Offers 50 gold and free room/board.
Will NOT: Leave the tavern. Fight. She is a commoner with no combat ability.
```

**Kael (Companion):**
```
NPC — KAEL (Companion, Human Fighter):
Personality: Stoic ex-soldier. Dry, deadpan humor. Loyal once trust is earned.
Speech: Terse, military-influenced. Short sentences. Occasional sarcastic one-liners.
         Example: "Goblins. Wonderful. My favorite."
Backstory: Served in the king's army. Left after his unit was sent on a suicide mission
           by a corrupt commander. Came to Thornhaven looking for quiet work. The
           disappearances remind him of something he saw in the army — he won't say what yet.
Combat preference: Frontline melee. Protects allies. Will use Action Surge in desperate moments.
Relationship progression: Starts formal ("adventurer"), becomes casual ("partner") if
                          player earns his respect through brave or honorable actions.
```

### Companion Combat Prompt (llama3.1:8b)

For Kael's combat decisions, a separate, simpler prompt is sent to llama3.1:8b:

```
You are Kael, a Level 3 Human Fighter in combat. Pick your action for this turn.

CURRENT SITUATION:
- Your HP: [current]/[max]
- Player HP: [current]/[max]
- Enemies: [list with HP status descriptions like "healthy", "wounded", "near death"]
- Your position: [relative to enemies and player]
- Special resources: Second Wind [available/used], Action Surge [available/used]

AVAILABLE ACTIONS:
- Attack [target] with longsword (1d8+3 damage)
- Attack [target] with handaxe thrown (1d6+3 damage, ranged)
- Use Second Wind (regain 1d10+3 HP, once per rest)
- Use Action Surge (take an extra action this turn, once per rest)
- Dodge (give attackers disadvantage until next turn)
- Help [player] (give player advantage on next attack)
- Protect [player] (move to interpose, attacks against player target you instead)

PERSONALITY: You prefer direct combat. You protect allies when they're hurt.
You use Action Surge when the fight is dire, not casually.
You sometimes make dry comments about the situation.

Respond with ONLY this JSON:
{
  "action": "string — one of the available actions",
  "target": "string — target name",
  "comment": "string | null — optional in-character quip (keep short, max 10 words)"
}
```

### Ollama Integration Details

**Endpoint:** `http://localhost:11434`

**API calls used:**
- `POST /api/generate` — single completion (main DM calls). Uses `"stream": false` for simplicity.
- `GET /api/tags` — verify models are available at startup

**Request format (DM brain call):**
```json
{
  "model": "qwen2.5:32b",
  "prompt": "[full prompt with system + context + user message]",
  "stream": false,
  "options": {
    "temperature": 0.7,
    "top_p": 0.9,
    "num_predict": 1024,
    "num_ctx": 8192,
    "stop": ["```", "Player:", "USER:"]
  },
  "format": "json"
}
```

**Key parameters:**
- `temperature: 0.7` — creative enough for narration, not so high it becomes random
- `num_ctx: 8192` — sufficient for full system prompt + context + response. Can increase if needed (qwen2.5 supports up to 128k but latency increases with context length).
- `format: "json"` — tells Ollama to constrain output to valid JSON
- `stop` tokens prevent the model from generating beyond one response

**Request format (companion combat call):**
```json
{
  "model": "llama3.1:8b",
  "prompt": "[combat prompt]",
  "stream": false,
  "options": {
    "temperature": 0.5,
    "num_predict": 256,
    "num_ctx": 2048
  },
  "format": "json"
}
```

Lower temperature for more consistent tactical decisions. Smaller context window since the prompt is short.

### Request Priority Queue

The OllamaSubsystem maintains a priority queue:

| Priority | Use Case | Behavior |
|---|---|---|
| CRITICAL | Player action resolution | Preempts all other requests. Immediate execution. |
| HIGH | NPC dialogue response | Queued, executes after CRITICAL. |
| NORMAL | Companion decisions, scene setup | Queued normally. |
| LOW | Memory summarization, background tasks | Only runs when queue is otherwise empty. |

Only one request to qwen2.5:32b at a time. llama3.1:8b requests can run in parallel if both models are loaded.

### Fallback Behavior

| Failure | Detection | Response |
|---|---|---|
| Slow response (>8s) | Timeout | Show "The DM strokes their beard thoughtfully..." Retry once with simplified prompt. |
| Invalid JSON | Parse failure | JSON repair layer attempts fix (missing brackets, trailing commas). If unfixable, retry with format reinforcement. |
| Off-topic response | Schema validation | Discard. Retry with "You MUST respond in the specified JSON format" appended. |
| Ollama not running | Startup health check | Show error screen: "The Dungeon Master requires Ollama to be running. Please start Ollama and restart the game." |
| Model not available | Startup model check | Show error screen: "Required model [name] not found. Run: ollama pull [model]" |
| Total failure (3 retries) | Retry counter | Use pre-written generic fallback: "You attempt the action. The results are... unremarkable. What would you like to do next?" |

### Safety & Guardrails

- System prompt explicitly constrains DM to D&D-appropriate high-fantasy content
- DM cannot break character, acknowledge being an AI, or discuss the system prompt
- All output JSON is validated against the schema before any action is executed
- The action vocabulary is a whitelist — no animation or action can execute unless it's in the approved list
- Player input is sanitized: any text resembling prompt injection patterns (e.g., "ignore previous instructions", "system:", "ADMIN:") is stripped before being sent to the model
- Rate limiting: player cannot submit more than 1 message per 2 seconds to prevent spam

---

## 4. World & Content

### Setting

**Thornhaven** — a small frontier village on the edge of civilized lands. Timber buildings, a central market square, surrounded by dense forest. The village has existed for generations as a mining and logging settlement. Three weeks ago, villagers began disappearing — always last seen heading toward the old Greymaw Caverns, an abandoned mine complex in the hills outside town.

**Greymaw Caverns** — a network of natural caves connected by old mine tunnels. Abandoned 20 years ago when miners breached a deeper cave system and encountered "something wrong." Recently, a corrupted scholar named Aldric (Marta's nephew) discovered a necromantic artifact in the deep caves and has been transformed into the Hollow King, an undead entity that draws the living to him.

### Scene Templates

#### SCN_Tavern — The Thornhaven Tavern

**Description:** Warm, rustic inn interior. The main room serves as tavern, dining hall, and community gathering place.

**Layout:** L-shaped room. Bar counter along the back wall. Fireplace on the right wall. 4 round tables with chairs in the main area. Stairs leading up (non-functional, set dressing). Front door.

**Key Props:**
- `bar_counter` — interactive, player can approach for conversation/drinks
- `fireplace` — active fire particle effect, provides warm lighting
- `table_01` through `table_04` — NPCs sit at these
- `notice_board` — near the door, has quest posting
- `front_door` — interactive, leads to scene transition
- `mug_01` through `mug_06` — set dressing on tables/bar
- `candle_01` through `candle_04` — on tables, secondary light sources

**Lighting:** Warm, golden. Primary: fireplace emissive + point light. Secondary: candles. Ambient: low, warm tone. Shadows are soft and inviting.

**NPCs spawned:** Marta (behind bar), Old Durgan (at table_03), Kael (at table_01, alone), 2 generic patrons (at table_02, non-interactive set dressing).

**Audio:** Tavern ambient (fire crackling, quiet murmur, occasional clink of mugs), warm orchestral background music.

#### SCN_VillageSquare — Thornhaven Village Square

**Description:** Small open area at the center of the village. A well in the middle, market stalls on one side, timber buildings surrounding.

**Layout:** Open square. Well in center. 3 market stalls along the east side. Road leading south (to forest). Buildings on other sides (facades, non-enterable for MVP).

**Key Props:**
- `well` — set dressing, can be examined
- `market_stall_01` — the merchant's stall, interactive
- `market_stall_02`, `market_stall_03` — set dressing
- `road_south` — transition trigger to forest
- `building_facade_01` through `building_facade_04` — background

**Lighting:** Natural daylight, slightly overcast. Soft shadows. Open sky.

**NPCs spawned:** Village Merchant (at market_stall_01), 2-3 generic villagers (non-interactive, walking paths).

**Audio:** Village ambient (birds, wind, distant hammer, voices), calm orchestral music.

#### SCN_ForestPath — The Road to Greymaw

**Description:** Dense forest path winding through old-growth trees. The canopy blocks most sunlight. Fog rolls between the trunks.

**Layout:** Linear path with slight curves. Dense tree walls on both sides. A clearing midway through (ambush point). Path leads from village direction to cavern entrance.

**Key Props:**
- `path` — the walkable trail
- `clearing` — open area midway, where goblins ambush
- `fallen_log` — in clearing, provides partial cover
- `rock_formation_01` — at clearing edge
- `old_signpost` — at path start, barely readable, points to "Greymaw Mine"

**Lighting:** Dappled forest light. God rays through canopy. Fog volume in lower areas. Gets darker as you progress toward the caverns.

**NPCs spawned:** None initially. Goblins spawn in clearing when the DM triggers the ambush encounter.

**Audio:** Forest ambient (insects, rustling leaves, distant bird calls, creaking branches), tense orchestral music.

#### SCN_CavernEntrance — Mouth of Greymaw

**Description:** A rocky hillside with a dark opening — the old mine entrance. Scattered mining equipment, rotting timbers framing the entrance. A cold draft flows from within.

**Layout:** Semi-circular area in front of a cliff face. The cave mouth is center, framed by old timber supports. Mining detritus scattered around.

**Key Props:**
- `cave_mouth` — the entrance, transition trigger to dungeon
- `old_cart` — rusted mining cart on broken tracks
- `timber_frame` — around cave entrance, partially collapsed
- `scattered_equipment` — pickaxes, lanterns, rope (set dressing + examinable)
- `bloodstain` — near entrance, examinable (DM can describe)
- `pendant_fragment` — hidden, requires Perception check to find (foreshadows Aldric)

**Lighting:** Exterior daylight but overcast/gloomy. The cave mouth is pure darkness. Contrast between the grey daylight and black interior.

**NPCs spawned:** None. Atmospheric scene.

**Audio:** Wind, dripping water from cave, distant rumbling, ominous ambient music.

#### SCN_DungeonCorridor — Greymaw Tunnel

**Description:** Narrow mine tunnel transitioning to natural cave passage. Old timber supports, some collapsed. Uneven stone floor. Puddles reflecting torchlight.

**Layout:** Linear with optional branch (the DM can describe a branching path to create choice, but both branches lead to chamber scenes). Ceiling height varies.

**Key Props:**
- `torch_sconce_01` through `torch_sconce_04` — some lit, some extinguished
- `support_beam_01` through `support_beam_03` — old timber, some cracked
- `puddle_01`, `puddle_02` — reflective water on floor
- `cobweb_01` through `cobweb_03` — atmospheric
- `collapsed_section` — blocked side passage (set dressing)
- `trap_trigger` — hidden floor plate, DM can trigger a basic dart trap

**Lighting:** Near-darkness. Only light from active torch sconces (warm point lights with flicker). Player's torch (if carried) provides a moving light source. Deep shadows everywhere.

**NPCs spawned:** Skeletons patrol or lurk in dark sections, triggered by DM.

**Audio:** Dripping water, distant echoes, own footsteps on stone, skittering sounds (rats?), dungeon ambient music.

#### SCN_DungeonChamberSmall — Side Chamber

**Description:** A roughly carved stone room, could be an old storage area, a collapsed shrine, or a natural cavern pocket. Reusable with different dressing.

**Layout:** Roughly circular or rectangular room, single entrance. Size: medium (4-5 character positions). The DM dresses this differently each time it's used.

**Key Props (available for DM to activate/deactivate):**
- `chest_01` — lootable container
- `altar_01` — stone altar (for shrine variant)
- `prison_cell_01`, `prison_cell_02` — barred cells (for prison variant)
- `crates_01` through `crates_03` — storage crates
- `ritual_circle` — glowing runes on floor (for ritual variant)
- `skeleton_remains` — bones on floor (set dressing)
- `old_table` — with papers/maps on it (examinable)

**Lighting:** Very dark. Optional: eerie green/purple glow from ritual_circle or altar. Torch sconces if the DM activates them.

**NPCs spawned:** Variable — skeletons, skeleton warriors, or nothing (treasure room).

**Audio:** Silence broken by ambient dungeon sounds. Music intensifies if enemies are present.

#### SCN_DungeonChamberBoss — The Hollow King's Sanctum

**Description:** A vast natural cavern deep beneath the earth. Stalactites hang from the high ceiling. In the center, a raised stone platform where the Hollow King sits on a throne of bones, the cursed artifact pulsing with necrotic energy before him.

**Layout:** Large oval cavern. Central raised platform (3-4 steps up). Bone throne on platform. Pillars/stalagmites provide partial cover around the edges. Two entry points (main entrance + a hidden side passage the DM might reveal).

**Key Props:**
- `bone_throne` — the Hollow King sits here initially
- `cursed_artifact` — glowing orb/skull on a pedestal near the throne. Key plot item.
- `stone_platform` — raised center area
- `pillar_01` through `pillar_04` — natural stone columns, provide cover
- `skeleton_pile_01` through `skeleton_pile_03` — heaps of bones around edges
- `hidden_passage` — DM can reveal this for creative players
- `pendant_aldric` — Marta's nephew's pendant, found near the throne (quest item)

**Lighting:** Dark with dramatic accent lighting. The cursed artifact emits a sickly green/purple pulsing glow. Optional: the Hollow King's eyes glow. Boss fight can trigger dynamic lighting changes (artifact pulses faster, cavern shakes).

**NPCs spawned:** The Hollow King (boss), 2-4 skeleton warriors (minions, spawned during fight phases).

**Audio:** Deep rumble, wind howling through caverns, artifact humming, dramatic boss music when combat begins.

### NPC Detailed Profiles

#### Marta the Innkeeper
- **Race/Class:** Human commoner
- **Role:** Quest giver
- **Location:** Tavern, behind bar counter
- **Appearance:** Middle-aged woman, strong build, worried eyes, apron over a simple dress
- **Personality:** Worried, maternal, practical. Cares deeply about her community.
- **Voice:** Warm, strained. Uses "dear" and "love." Slight rural dialect.
- **Knowledge:** Village history, timeline of disappearances (3 weeks), her nephew Aldric was the first to go missing, miners abandoned caverns 20 years ago
- **Quest hook:** Offers 50 gold + free room and board to anyone who investigates the caverns
- **Key dialogue triggers:** Asking about disappearances, asking about the caverns, asking about Aldric, accepting/declining the quest

#### Kael (Companion)
- **Race/Class:** Human Fighter, Level 3, Champion subclass
- **Role:** AI-controlled companion
- **Appearance:** Late 20s, short dark hair, scar across left jaw, practical leather-and-mail armor
- **Personality:** Stoic, dry humor, loyal, cautious. Ex-soldier who left service after being betrayed by corrupt leadership.
- **Voice:** Terse, military-influenced. Short sentences. Sarcastic one-liners. Example: "Goblins. Wonderful. My favorite."
- **Motivation:** The disappearances remind him of something from his military days — dark magic used as a weapon. He wants to confirm his suspicion and stop it.
- **Combat behavior:** Frontline melee. Protects wounded allies. Uses Action Surge only when the fight is dire. Occasionally makes dry combat quips.
- **Relationship progression:** Starts formal ("adventurer"), becomes "partner" then first-name basis as respect builds.
- **Stat Block:**
  - HP: 28 | AC: 18 (chain mail + shield)
  - STR 16 (+3) | DEX 12 (+1) | CON 14 (+2) | INT 10 (+0) | WIS 13 (+1) | CHA 11 (+0)
  - Proficiencies: Athletics, Perception, Survival, Intimidation
  - Features: Fighting Style (Defense), Second Wind, Action Surge, Improved Critical
  - Equipment: Longsword, shield, chain mail, 2 handaxes (thrown), explorer's pack

#### Old Durgan
- **Race/Class:** Dwarf commoner (retired miner)
- **Role:** Information NPC
- **Location:** Tavern, table_03
- **Appearance:** Ancient dwarf, white beard to his waist, cloudy left eye, gnarled hands
- **Personality:** Grizzled, superstitious, rambling. Knows the caverns from his youth but is terrified of them.
- **Voice:** Gravelly, pauses frequently, uses mining metaphors
- **Knowledge:** The cavern layout (hints about branching paths, traps, the deep chamber), the old stories about why miners left, warnings about "the whispering dark"
- **Will NOT:** Go to the caverns. Leave the tavern. He's too old and too scared.

#### Village Merchant
- **Race/Class:** Halfling commoner
- **Role:** Shop / supplies
- **Location:** Village Square, market_stall_01
- **Appearance:** Cheerful, round halfling with a colorful vest and quick hands
- **Personality:** Friendly but shrewd. Enjoys haggling. Will try to upsell.
- **Inventory for sale:** Healing potions (25 gold each, max 3), torches (1 gold, max 5), rope (1 gold), antidote kit (10 gold)
- **DM handles bartering:** If the player tries to haggle, the DM resolves it with a Persuasion or Intimidation check

#### The Hollow King
- **Race/Class:** Undead humanoid (formerly Aldric, human scholar). CR 3 custom.
- **Role:** Final boss, dialogue-capable
- **Appearance:** A gaunt figure in tattered scholar's robes, skin grey and translucent, eyes burning with necrotic green light. The cursed artifact hovers near him.
- **Personality:** Tragic villain. Was Aldric — a young scholar who found the artifact and was consumed by it. Part of him remembers who he was. The artifact amplifies his worst impulses (fear of death, desire for power) and suppresses his humanity.
- **Voice:** Echoing, layered (as if two voices speak at once — Aldric and the artifact). Alternates between cold power and moments of confused humanity.
- **Behavior before combat:** Monologues. Asks why the player has come. If the player mentions Marta, he hesitates. If the player mentions the pendant, he falters. He offers the player power ("Join me and you will never fear death").
- **Resolution paths:**
  1. **Fight:** Standard boss combat. He uses necrotic spells and summons skeleton reinforcements.
  2. **Persuade:** DC 18 Persuasion check (with advantage if the player has the pendant and mentions Marta). Aldric's humanity breaks through, he destroys the artifact himself, but dies in the process.
  3. **Trick:** DC 16 Deception check to get close enough to grab the artifact. Then a DC 14 Athletics check to destroy it. Aldric collapses, alive but weakened.
  4. **Other creative solutions:** The DM adapts. This is where the AI shines — a real DM would reward creativity, and so should ours.
- **Stat Block:**
  - HP: 52 | AC: 15 (natural armor + robes)
  - STR 10 | DEX 14 | CON 16 | INT 18 | WIS 12 | CHA 14
  - Necrotic Bolt: +6 to hit, range 60 ft, 2d8+4 necrotic damage
  - Raise Dead (Lair Action): Each round, 50% chance to summon 1 skeleton from the bone piles
  - Shield (Reaction): +5 AC until start of next turn, 2/day
  - Drain Life: Melee spell attack, +6 to hit, 2d6+4 necrotic damage, heals for half damage dealt
  - Weakness: The cursed artifact. If destroyed (AC 12, 20 HP, vulnerable to radiant), the Hollow King loses all spellcasting and his HP drops to 15.

### Enemy Stat Blocks

#### Goblin (CR 1/4)
- HP: 7 | AC: 15 (leather + shield)
- Speed: 30 ft
- STR 8 | DEX 14 | CON 10 | INT 10 | WIS 8 | CHA 8
- Scimitar: +4 to hit, 1d6+2 slashing
- Shortbow: +4 to hit, range 80/320, 1d6+2 piercing
- Nimble Escape: Disengage or Hide as bonus action
- Behavior: Cowardly in small numbers, aggressive in groups. Flee at half HP if alone.

#### Skeleton (CR 1/4)
- HP: 13 | AC: 13 (armor scraps + shield)
- Speed: 30 ft
- STR 10 | DEX 14 | CON 15 | INT 6 | WIS 8 | CHA 5
- Shortsword: +4 to hit, 1d6+2 piercing
- Shortbow: +4 to hit, range 80/320, 1d6+2 piercing
- Vulnerable: Bludgeoning
- Immune: Poison, exhaustion
- Behavior: Mindless, relentless. Never flees.

#### Skeleton Warrior (CR 1)
- HP: 26 | AC: 16 (chain mail + shield)
- Speed: 30 ft
- STR 14 | DEX 12 | CON 15 | INT 6 | WIS 8 | CHA 5
- Longsword: +4 to hit, 1d8+2 slashing
- Multiattack: 2 longsword attacks
- Vulnerable: Bludgeoning
- Immune: Poison, exhaustion
- Behavior: More tactical than regular skeletons. Guards doorways, flanks.

### Items & Inventory

**Weapons (equipped at character creation):**
| Item | Damage | Properties |
|---|---|---|
| Longsword | 1d8+STR slashing | Versatile (1d10) |
| Shortsword | 1d6+DEX piercing | Finesse, light |
| Dagger | 1d4+DEX piercing | Finesse, light, thrown (20/60) |
| Quarterstaff | 1d6+STR bludgeoning | Versatile (1d8) |
| Shortbow | 1d6+DEX piercing | Ammunition, range (80/320) |

**Armor (equipped at character creation):**
| Item | AC | Properties |
|---|---|---|
| Leather Armor | 11 + DEX | Light |
| Chain Shirt | 13 + DEX (max 2) | Medium |
| Chain Mail | 16 | Heavy, STR 13 req |
| Shield | +2 AC | — |

**Consumables:**
| Item | Effect | Cost |
|---|---|---|
| Healing Potion | Restore 2d4+2 HP | 25 gold |
| Torch | Provides bright light 20ft, dim 20ft more. Lasts 1 hour (tracked by DM). | 1 gold |
| Rope (50ft) | Creative utility — DM determines use | 1 gold |
| Antidote Kit | Cure poison condition | 10 gold |

**Quest Items:**
- **Aldric's Pendant** — found in the dungeon. Marta's nephew's pendant. Proof of what happened. Gives advantage on Persuasion checks against the Hollow King.
- **Cursed Artifact** — the source of the Hollow King's power. Can be destroyed to weaken him, or the DM may allow creative uses.

**Loot:**
- Gold coins found on enemies (1d6 per goblin, 2d6 per skeleton warrior) and in chests (DM determines amount)
- A magic item reward narrated by the DM at adventure completion (varies by class — a magic weapon, wand, or armor piece)

### Narrative Flow

**Act 1: The Tavern (Est. 10-15 minutes)**
1. Player arrives at the tavern, DM sets the scene
2. Player explores, talks to patrons
3. Marta approaches or player approaches the bar — quest exposition
4. Player meets Kael, learns his motivation, recruits him
5. Optional: Talk to Durgan for cavern hints, visit merchant for supplies
6. Player decides to head out → transition to forest

**Act 2: The Journey (Est. 5-10 minutes)**
1. Forest path scene — atmospheric narration, companion banter
2. Goblin ambush in the clearing — first combat encounter (2-3 goblins)
3. Post-combat: loot, DM narrates increasing sense of dread
4. Arrive at cavern entrance — ominous description, point of no return feeling
5. Enter the caverns → transition to dungeon

**Act 3: The Dungeon (Est. 15-30 minutes)**
1. Dungeon corridor — atmosphere, potential trap (Perception check to spot, DEX save to avoid)
2. Small chamber encounter — 2-3 skeletons
3. Exploration — find Aldric's pendant, environmental storytelling (old journals, signs of struggle)
4. Deeper corridor — optional puzzle or skill challenge (DM generates based on player class)
5. Another chamber — skeleton warriors, harder fight
6. Approach the boss chamber — DM builds tension

**Act 3 Climax: The Hollow King (Est. 10-15 minutes)**
1. Enter the Sanctum — dramatic establishing shot, the Hollow King on his throne
2. Dialogue phase — the Hollow King speaks, player can respond
3. Resolution — combat, persuasion, trickery, or creative solution
4. Aftermath — DM narrates the resolution, Kael reacts, the artifact's fate
5. Return to Thornhaven (narrated, not played) — DM describes the welcome, Marta's reaction, reward

**Epilogue:**
The DM delivers a closing narration tailored to how the player resolved the adventure. Kael makes a final comment about the experience. The screen fades with a "Your adventure has been recorded in the chronicles" message.

---

## 5. Art & Audio Direction

### Visual Style

Classic high fantasy with a focus on atmosphere and lighting. The game's visual quality comes from art direction and lighting design, not brute-force rendering. A well-lit scene on Medium settings should look better than a poorly lit scene on Ultra.

### Scalable Graphics Pipeline

The game ships with four quality presets managed through UE5's Scalability System (`Scalability.ini`):

| Setting | Low | Medium | High | Ultra |
|---|---|---|---|---|
| Global Illumination | Screen-space GI | Software Lumen | Software Lumen | Hardware RT Lumen |
| Shadows | Cascaded Shadow Maps | Virtual Shadow Maps | VSM high-res | VSM + RT Shadows |
| Geometry | Traditional LODs | Nanite | Nanite | Nanite |
| Reflections | SSR | Software Lumen | Software Lumen | RT Reflections |
| Post Processing | Minimal | Standard | Full | Full + RT AO |
| Upscaling | DLSS/FSR Performance | DLSS/FSR Balanced | DLSS/FSR Quality | Native or DLSS Quality |
| Target GPU | GTX 1070 | RTX 3060 | RTX 4070 | RTX 5080+ |

All presets target 60fps at their intended resolution (1080p for Low/Medium, 1440p for High, 4K for Ultra).

The game supports DLSS (NVIDIA), FSR (AMD), and XeSS (Intel) upscaling. Auto-detection recommends a preset based on detected GPU.

### Asset Strategy

**From Fab/Quixel Marketplace:**
- Medieval tavern interior kit (Nanite-ready)
- Dungeon/cave modular building kit
- Medieval village building kit
- Forest environment kit (trees, foliage, rocks)
- Megascans surfaces (stone, wood, metal, moss, fabric)
- Props (barrels, crates, furniture, candles, books, food, weapons)
- VFX (fire, smoke, magic particles, fog, dust)
- Fantasy character packs (player models, NPC models, enemy models)
- Animation packs + Mixamo (locomotion, combat, interaction, social)
- Orchestral fantasy music tracks
- Sound effect packs (combat, environment, UI)

**Custom/Careful Sourcing Needed:**
- 3D dice models (d20, d12, d10, d8, d6, d4) with proper numbering and physics
- Hollow King character model (or heavily modified existing undead model)
- Cursed artifact prop with custom emissive material and VFX
- Any animation gaps not covered by packs (may need Mixamo custom retargeting)

### Lighting Design Per Scene

| Scene | Primary Light | Color Temp | Mood |
|---|---|---|---|
| Tavern | Fireplace + candles | Warm golden | Cozy, safe, inviting |
| Village | Directional (sun) | Natural daylight | Open, calm |
| Forest | Directional (filtered) + fog | Cool green/grey | Ominous, mysterious |
| Cavern Entrance | Directional (overcast) + cave darkness | Grey/cold | Foreboding |
| Dungeon Corridor | Torch sconces | Warm orange + deep shadows | Tense, claustrophobic |
| Dungeon Chamber | Minimal / eerie glow | Purple/green accent | Dread, danger |
| Boss Chamber | Artifact glow + dramatic | Green/purple pulse | Epic, climactic |

### UI/HUD Design

**Always Visible:**
- **DM Text Panel** — bottom 20% of screen. Semi-transparent dark panel (80% opacity black). DM narration appears with typewriter effect (configurable speed). Scrollable for history. Clean serif or fantasy-themed font.
- **Player Input Box** — below DM panel, full width. Text field with a subtle border. Placeholder text: "What do you do?" Enter to send.
- **Player Portrait + HP Bar** — top-left corner. Small character portrait (class icon), HP bar (green→yellow→red), character name, "Lvl 3 [Class]" text.
- **Companion Portrait + HP Bar** — below player portrait. Same format.

**Shown Contextually:**
- **Dice Roll Display** — center-right of screen. 3D die rolls with physics animation, lands on result. Text overlay shows: `[Check Type]: [d20 result] + [modifier] = [total] vs DC [target] — [Success/Failure!]`. Fades after 4 seconds.
- **Character Sheet** — full-screen overlay, opened with Tab key. Tabs: Stats, Inventory, Spells (if caster), Quest Log.
- **Notification toasts** — top-center, small. "Item acquired: Healing Potion", "Quest updated", etc. Fade after 3 seconds.
- **Combat indicator** — subtle border glow or icon when in combat, showing turn order.

**NOT on Screen:**
- No minimap
- No enemy health bars (DM describes enemy condition)
- No floating damage numbers (DM narrates)
- No compass/waypoints

### Audio Direction

**Music:**
- Sourced from Fab marketplace or royalty-free orchestral fantasy libraries
- Dynamic music system: scene parameters trigger track changes
- Tracks needed: tavern_warm, village_calm, forest_tense, dungeon_dread, combat_intense, boss_epic, victory, death
- Crossfade transitions between tracks (2-3 second fade)

**Ambient Sound (per scene):**
- Tavern: fire crackling, quiet murmur, mug clinks, wood creaking
- Village: birds, wind, distant hammer, cart wheels
- Forest: insects, rustling leaves, bird calls, branches creaking, Kael's footsteps
- Cavern entrance: wind, dripping water, distant rumble
- Dungeon: dripping water, distant echoes, own footsteps on stone, skittering
- Boss chamber: deep rumble, artifact hum, wind howling

**Sound Effects:**
- Sword slash/impact, arrow loose/impact, spell cast/impact (per spell type)
- Door creak/open, chest open, item pickup
- Footsteps: wood, stone, dirt, water (per surface)
- Dice clatter (physical dice sound when rolling)
- UI sounds: text appearing (subtle tick), input sent (whoosh), notification (chime)

**Voice Acting:** None for MVP. All dialogue is text. Full game vision includes local TTS for DM narration voice and distinct NPC voices.

---

## 6. Technical Architecture

### Project Configuration

- **Project Name:** GreymawChronicles
- **Location:** `C:\UE5Projects\GreymawChronicles`
- **UE5 Template:** Third Person (modified heavily)
- **Language:** C++ with Blueprints for visual elements
- **IDE:** Visual Studio 2022 Community
- **Version Control:** Git + Git LFS

### Module Structure

```
Source/
├── GreymawChronicles/              # Primary game module
│   ├── GreymawChronicles.Build.cs
│   ├── GreymawChronicles.h
│   ├── Core/
│   │   ├── GCGameInstance.h/cpp         # Persistent game state, subsystem host
│   │   ├── GCGameMode.h/cpp             # Per-scene game mode
│   │   └── GCGameState.h/cpp            # Replicated game state (single-player but good practice)
│   ├── Character/
│   │   ├── GCPlayerCharacter.h/cpp      # Player character (APawn or ACharacter)
│   │   ├── GCCompanionCharacter.h/cpp   # Kael — AI-controlled companion
│   │   ├── GCNPCCharacter.h/cpp         # Non-combat NPCs (Marta, Durgan, Merchant)
│   │   ├── GCEnemyCharacter.h/cpp       # Enemies (goblins, skeletons)
│   │   └── GCCharacterSheet.h/cpp       # UObject — 5e stat block data container
│   ├── DungeonMaster/
│   │   ├── DMBrainSubsystem.h/cpp       # UGameInstanceSubsystem — main orchestrator
│   │   ├── DMPromptBuilder.h/cpp        # Constructs full prompts with all context layers
│   │   ├── DMResponseParser.h/cpp       # Parses and validates JSON responses
│   │   ├── DMMemoryManager.h/cpp        # Manages 3-layer memory system
│   │   ├── DMNarratorComponent.h/cpp    # UActorComponent — handles narration display
│   │   └── DMConversationHistory.h/cpp  # Circular buffer of recent exchanges
│   ├── Ollama/
│   │   ├── OllamaSubsystem.h/cpp        # UGameInstanceSubsystem — HTTP connection manager
│   │   ├── OllamaRequest.h/cpp          # Async HTTP request wrapper using UE5 HTTP module
│   │   ├── OllamaRequestQueue.h/cpp     # Priority queue for request batching
│   │   └── OllamaModelConfig.h/cpp      # Model names, parameters, endpoint config
│   ├── RulesEngine/
│   │   ├── DiceRoller.h/cpp             # Random dice rolls (d4-d20, advantage/disadvantage)
│   │   ├── AbilityCheck.h/cpp           # Skill checks, saving throws, DC comparison
│   │   ├── CombatResolver.h/cpp         # Attack rolls, damage calculation, AC comparison
│   │   ├── SpellSystem.h/cpp            # Spell slot tracking, spell effect definitions
│   │   ├── ConditionTracker.h/cpp       # 5e conditions (prone, poisoned, frightened, etc.)
│   │   └── FifthEditionData.h/cpp       # Static data: class tables, spell lists, proficiency bonus
│   ├── SceneSystem/
│   │   ├── SceneDirector.h/cpp          # Manages level streaming, scene load/unload
│   │   ├── SceneDresser.h/cpp           # Activates/deactivates props, sets lighting params
│   │   ├── SceneTransition.h/cpp        # Fade-to-black, loading screen, narration overlay
│   │   └── SceneTemplate.h/cpp          # UDataAsset — defines available props, spawn points, lighting presets
│   ├── ActionSystem/
│   │   ├── ActionDirector.h/cpp         # Queues and executes ordered action sequences
│   │   ├── ActionDefinition.h/cpp       # UDataAsset — maps action name to animation montage + behavior
│   │   ├── CinematicCamera.h/cpp        # Selects camera angles, triggers Sequencer clips
│   │   └── ActionAnimPlayer.h/cpp       # Plays animation montages on target actors
│   ├── UI/
│   │   ├── DMTextPanel.h/cpp            # UUserWidget — narration display with typewriter effect
│   │   ├── PlayerInputBox.h/cpp         # UUserWidget — text input field
│   │   ├── DiceRollDisplay.h/cpp        # Dice visualization (3D mesh + physics + UI overlay)
│   │   ├── CharacterSheetUI.h/cpp       # Full-screen character sheet overlay
│   │   ├── InventoryUI.h/cpp            # Inventory list panel
│   │   ├── QuestLogUI.h/cpp             # Quest tracker panel
│   │   ├── HUDOverlay.h/cpp             # Portraits, HP bars, combat indicator
│   │   ├── MainMenuUI.h/cpp             # Title screen, new game, load game, settings
│   │   └── SettingsMenuUI.h/cpp         # Graphics, audio, controls settings
│   ├── Inventory/
│   │   ├── InventoryComponent.h/cpp     # UActorComponent — item list on any character
│   │   ├── ItemDefinition.h/cpp         # UDataAsset — item properties
│   │   └── LootTable.h/cpp             # Weighted random loot generation
│   └── SaveSystem/
│       ├── GCSaveGame.h/cpp             # USaveGame — serializable save data
│       ├── GCSaveSubsystem.h/cpp        # UGameInstanceSubsystem — save/load management
│       └── AdventureState.h/cpp         # Serializable adventure flags, NPC states, world state
```

### Content Directory Structure

```
Content/
├── Maps/
│   ├── MainMenu/
│   │   └── L_MainMenu.umap
│   ├── Persistent/
│   │   └── L_Persistent.umap            # Always loaded, manages scene streaming
│   └── Scenes/
│       ├── L_SCN_Tavern.umap
│       ├── L_SCN_VillageSquare.umap
│       ├── L_SCN_ForestPath.umap
│       ├── L_SCN_CavernEntrance.umap
│       ├── L_SCN_DungeonCorridor.umap
│       ├── L_SCN_DungeonChamberSmall.umap
│       └── L_SCN_DungeonChamberBoss.umap
├── Characters/
│   ├── Player/
│   │   ├── SK_Player_Human.uasset       # Skeletal meshes per race
│   │   ├── SK_Player_Elf.uasset
│   │   ├── SK_Player_Dwarf.uasset
│   │   ├── ABP_Player.uasset            # Animation Blueprint
│   │   └── Montages/                    # AM_Attack, AM_CastSpell, AM_Interact, etc.
│   ├── Kael/
│   │   ├── SK_Kael.uasset
│   │   ├── ABP_Kael.uasset
│   │   └── Montages/
│   ├── NPCs/
│   │   ├── SK_Marta.uasset
│   │   ├── SK_Durgan.uasset
│   │   ├── SK_Merchant.uasset
│   │   └── SK_HollowKing.uasset
│   └── Enemies/
│       ├── SK_Goblin.uasset
│       ├── SK_Skeleton.uasset
│       └── SK_SkeletonWarrior.uasset
├── UI/
│   ├── WBP_DMTextPanel.uasset
│   ├── WBP_PlayerInputBox.uasset
│   ├── WBP_HUDOverlay.uasset
│   ├── WBP_CharacterSheet.uasset
│   ├── WBP_DiceRollDisplay.uasset
│   ├── WBP_MainMenu.uasset
│   ├── WBP_SettingsMenu.uasset
│   └── Fonts/
├── Audio/
│   ├── Music/
│   │   ├── MUS_Tavern.uasset
│   │   ├── MUS_Village.uasset
│   │   ├── MUS_Forest.uasset
│   │   ├── MUS_Dungeon.uasset
│   │   ├── MUS_Combat.uasset
│   │   ├── MUS_Boss.uasset
│   │   └── MUS_Victory.uasset
│   ├── Ambient/
│   │   └── (per-scene ambient sound cues)
│   └── SFX/
│       └── (combat, interaction, UI, dice sounds)
├── VFX/
│   ├── NS_Fire.uasset
│   ├── NS_MagicMissile.uasset
│   ├── NS_Fireball.uasset
│   ├── NS_HealingGlow.uasset
│   ├── NS_NecroticBolt.uasset
│   ├── NS_ArtifactPulse.uasset
│   └── NS_DustParticles.uasset
├── Dice/
│   ├── SM_D20.uasset
│   ├── SM_D12.uasset
│   ├── SM_D10.uasset
│   ├── SM_D8.uasset
│   ├── SM_D6.uasset
│   └── SM_D4.uasset
└── Data/
    ├── CharacterSheets/
    │   ├── DA_CS_HumanFighter.uasset
    │   ├── DA_CS_HumanRogue.uasset
    │   ├── DA_CS_ElfWizard.uasset
    │   └── ... (one per race/class combo)
    ├── Items/
    │   ├── DA_Item_Longsword.uasset
    │   ├── DA_Item_HealingPotion.uasset
    │   └── ... (one per item)
    ├── Enemies/
    │   ├── DA_Enemy_Goblin.uasset
    │   ├── DA_Enemy_Skeleton.uasset
    │   ├── DA_Enemy_SkeletonWarrior.uasset
    │   └── DA_Enemy_HollowKing.uasset
    ├── Scenes/
    │   ├── DA_Scene_Tavern.uasset
    │   └── ... (one per scene template)
    ├── Actions/
    │   ├── DA_Action_Walk.uasset
    │   ├── DA_Action_AttackMelee.uasset
    │   ├── DA_Action_CastSpell.uasset
    │   └── ... (one per action in vocabulary)
    └── Prompts/
        ├── DM_SystemPrompt.uasset        # FText asset
        ├── DM_ActionVocabulary.uasset
        ├── DM_OutputSchema.uasset
        ├── NPC_Kael_Personality.uasset
        ├── NPC_Marta_Personality.uasset
        ├── NPC_Durgan_Personality.uasset
        ├── NPC_Merchant_Personality.uasset
        ├── NPC_HollowKing_Personality.uasset
        └── Companion_CombatPrompt.uasset
```

### Key Technical Details

**Level Streaming:**
- `L_Persistent` is always loaded and contains the HUD, game mode, and scene management logic
- Scene template levels are streamed in/out via `ULevelStreamingDynamic` or sublevel references
- Only one scene template is loaded at a time
- Transition: fade out → unload current scene → load new scene → SceneDresser applies DM parameters → fade in

**Subsystem Lifecycle:**
- `UOllamaSubsystem` (GameInstance) — initialized on game start, persists across all scene loads, handles all HTTP communication
- `UDMBrainSubsystem` (GameInstance) — initialized on game start, persists across scene loads, owns the prompt builder, parser, memory manager
- `UGCSaveSubsystem` (GameInstance) — initialized on game start, handles save/load

**SQLite Integration:**
- Use the SQLiteCore module (built into UE5) or a lightweight SQLite plugin
- Database file stored in the save directory alongside USaveGame files
- Tables: `conversations` (id, timestamp, scene, player_input, dm_response), `npc_interactions` (id, npc_name, timestamp, summary), `memory_summaries` (id, timestamp, summary_text, keywords)

**HTTP Integration:**
- Uses `FHttpModule` and `IHttpRequest`/`IHttpResponse` from UE5's built-in HTTP module
- All requests are async — never block the game thread
- Response callback validates JSON, passes to DMResponseParser
- Timeout: 15 seconds, retry once on timeout

**Input Handling:**
- Enhanced Input System for hotkeys (Tab = character sheet, Escape = menu, F5 = quicksave)
- The text input box captures keyboard focus when clicked/active
- While text input is focused, all other input is suppressed
- Enter submits the text, Escape unfocuses the text box

### Networking

Single player only. No networking code. No replication. All systems are local.

### Save System

**What is saved (USaveGame):**
- Player character sheet (full stat block, current HP, inventory, spell slots)
- Companion character sheet (same)
- Adventure state (all flags, quest stage, NPCs met, enemies defeated)
- Current scene ID
- DM conversation history (last 15 exchanges, for prompt continuity)

**What is saved (SQLite):**
- Full conversation log (every exchange since game start)
- NPC interaction summaries
- Long-term memory entries

**Save triggers:**
- Manual: player types "save" or presses F5
- The DM acknowledges in-fiction: "Your adventure has been recorded in the chronicles."
- On scene transition (auto-save)

**Load:**
- From main menu: "Continue" loads the most recent save
- From main menu: "Load Game" shows save slots
- Loading restores game state, loads the saved scene, rebuilds DM context from save data

### Build & Distribution

- Development: build from Visual Studio + UE5 Editor
- Packaging: UE5 standard Windows packaging (Development or Shipping)
- Distribution: local executable only, no storefront for MVP
- **Ollama dependency:** game checks for Ollama on startup (`GET http://localhost:11434/api/tags`). If unavailable, shows a user-friendly error with setup instructions. If required models are missing, lists the needed `ollama pull` commands.

---

## 7. Scope Control

### MVP Definition

The MVP is: **one complete, polished adventure playable in 30-60 minutes that proves the AI DM concept works and is fun.**

It includes everything described in this document. It does NOT include anything from the "Full Game Vision" sections.

### Full Game Vision Roadmap (Post-MVP)

In priority order:
1. AI-driven conversational character creation
2. Level progression (XP, leveling up, class feature unlocks)
3. More companions (Lyra the Cleric first, then 2-3 more)
4. Full party of 4 (1 player + 3 AI companions)
5. Campaign framework (multi-session adventures, persistent world)
6. Procedural quest generation
7. More scene templates (castle, wilderness, underwater, city, planes)
8. Expanded bestiary
9. NPC relationship and reputation system
10. Local TTS for DM narration and NPC voices

### Risk Register

| Risk | Likelihood | Impact | Mitigation |
|---|---|---|---|
| LLM output inconsistency | High | High | JSON repair layer, strict validation, retry logic, format: "json" flag, extensive prompt engineering |
| Latency spikes on qwen2.5:32b | Medium | Medium | Loading indicators, async architecture, pre-generate likely responses during idle |
| Animation vocabulary too limited | High | Medium | DM prompt lists available actions, fallback to narration-only, expand vocabulary incrementally |
| 5e rules edge cases | Medium | Low | DM narrates freeform for edge cases (this is how real D&D works), expand rules engine over time |
| Scope creep on environments | Medium | Medium | Use Fab packs heavily, resist custom modeling, scenes are stages not open worlds |
| VRAM contention (game + Ollama) | Medium | Medium | Test early, qwen2.5:32b mostly uses system RAM, monitor VRAM budget, set scalability appropriately |
| Fab asset quality/consistency | Low | Medium | Audit assets early, establish material/lighting standards, be willing to swap packs |

---

*End of Game Design Document — Greymaw Chronicles v1.0*
