# Greymaw Chronicles — Orchestrator Instructions
## How to Use These Documents with Claude Code Agent Teams

---

## Overview

You are the orchestrator for building **Greymaw Chronicles**, an AI-powered D&D game in Unreal Engine 5.7. You have four reference documents:

1. **GreymawChronicles_GDD.md** — The complete Game Design Document. This describes WHAT the game is. Agent teams should read relevant sections before coding.
2. **GreymawChronicles_AgentRoadmap_Part1.md** — Implementation tasks for Milestones 1-6.
3. **GreymawChronicles_AgentRoadmap_Part2.md** — Implementation tasks for Milestones 7-22.
4. **GreymawChronicles_OllamaSetup.md** — Ollama model setup. Run this BEFORE starting any development.

## Pre-Development Setup

Before assigning any development tasks:

1. **Run the Ollama setup** — Follow `GreymawChronicles_OllamaSetup.md` to create the custom model configurations and verify all models are working.
2. **Read the full GDD** — Understand the complete game design before starting.
3. **Understand the dependency graph** from the Agent Roadmap Part 2 (bottom of file).

## Orchestration Workflow

### Step 1: Find the Next Available Task

Scan the Agent Roadmap files for tasks where:
- `Status: NOT_STARTED`
- All tasks listed in `Dependencies` have `Status: COMPLETE`

Multiple tasks can run in parallel if they don't share dependencies.

### Step 2: Assign to Agent Team

Each task specifies a **Team** label. This is a suggested grouping — you can assign to any available agent. Provide the agent with:
- The task instructions from the roadmap
- The relevant GDD sections (tell the agent which sections to read)
- The project location: `C:\UE5Projects\GreymawChronicles`
- The expected output files

### Step 3: Verify Completion

When an agent reports a task is done:
- Check every acceptance criteria checkbox
- If any criteria fails, send the task back with specific feedback
- Only mark `COMPLETE` when ALL criteria pass

### Step 4: Update Status

Mark the task as `COMPLETE` in your tracking. Move to the next available task.

### Step 5: Repeat

Continue until all milestones are complete. The critical path is:

**M1 → M2 → M3 → M6 → M9 → M14 → M17 → M22**

Prioritize tasks on this critical path. Parallelizable work (like building scenes in M15 while combat is being developed in M14) should be done simultaneously when possible.

## Agent Team Roles

| Team Label | Responsibilities |
|---|---|
| **infrastructure** | Project setup, Git, build system, scalability config |
| **ai-integration** | Ollama HTTP layer, request queue, health checks |
| **ai-core** | DM brain, prompt building, response parsing, memory |
| **gameplay** | 5e rules engine, character sheets, inventory, combat, save/load |
| **ui** | All UMG widgets — text panel, input box, HUD, menus, dice display |
| **scene** | Scene system, level streaming, scene templates, transitions |
| **environment** | Building actual scene levels with Fab assets, lighting, props |
| **character** | Character models, animations, montages, Animation Blueprints |
| **action** | ActionDirector, CinematicCamera, action-to-animation pipeline |
| **audio** | Music, ambient sound, SFX |
| **integration** | Cross-system wiring, end-to-end testing |
| **design** | Adventure scripting, prompt tuning, playtesting |

## Key Decision Points

### After Milestone 6 (The Talking Demo)
This is the first time the AI DM is testable. **STOP and playtest.** Spend time:
- Testing various player inputs
- Tuning the DM system prompt for quality responses
- Verifying JSON output reliability
- Checking latency on the target hardware
- Fixing any prompt engineering issues

If the DM responses are poor, fix them HERE before building the 3D systems. The AI brain is the foundation — everything depends on it working well.

### After Milestone 9 (Action Translation)
This is the proof of concept. A player types a command and a 3D character performs it. **STOP and evaluate.** Does it feel magical? Is the translation from text to action convincing? If not, this is where to iterate on the action vocabulary and animation system.

### After Milestone 17 (The Adventure)
This is the full MVP. **Extended playtest.** Play through multiple times, trying different approaches:
- Play as each class
- Try to break the DM with unusual requests
- Test all resolution paths for the boss encounter
- Verify save/load mid-adventure
- Time the full run (should be 30-60 minutes)

Document everything that needs fixing for Milestone 22 (Polish).

## Important Technical Notes

- **All code is C++ with Blueprints for visual elements** (UI widgets, animation BPs)
- **UE5 version is 5.7.3** — use appropriate API versions
- **Project path:** `C:\UE5Projects\GreymawChronicles`
- **IDE:** Visual Studio 2022 Community
- **Ollama runs at:** `http://localhost:11434`
- **Models:** qwen2.5:32b (main DM brain), llama3.1:8b (companion/memory)
- **Custom model aliases:** greymaw-dm, greymaw-companion, greymaw-memory
- **Git + Git LFS** is configured for version control
- **Single player only** — no networking code needed

## Error Recovery

If an agent produces code that doesn't compile:
1. Read the error messages carefully
2. Check if a dependency was missed
3. Verify the correct UE5 module is included in Build.cs
4. Check if the agent used deprecated API

If the DM produces poor responses:
1. Check the system prompt for clarity
2. Verify all context sections are being included
3. Test the same prompt directly via curl to isolate game code vs model issues
4. Consider reducing context size if latency is too high

If VRAM issues occur:
1. Check nvidia-smi for usage
2. Lower game graphics preset
3. Reduce num_ctx in Ollama requests
4. Ensure only needed models are loaded
