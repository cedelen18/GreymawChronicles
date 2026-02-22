# Greymaw Chronicles — Ollama Model Setup Guide

## Currently Installed Models

Verified via `ollama list`:

| Model | Size | Status |
|---|---|---|
| qwen2.5:32b | 19 GB | ✅ Installed |
| qwen2.5vl:32b | 21 GB | ✅ Installed (not needed for this game) |
| llama3.1:8b | 4.9 GB | ✅ Installed |

## Model Recommendation Table

| Game System | Model | Size | Why | Latency Target | Installed? | Action |
|---|---|---|---|---|---|---|
| DM Narration & Storytelling | qwen2.5:32b | 19GB | Strong creative writing, consistent tone, excellent instruction following | <3s | ✅ Yes | None |
| NPC Dialogue & Personality | qwen2.5:32b | 19GB | Good roleplay, maintains character voice with system prompts | <2s | ✅ Yes | None |
| Scene Direction (JSON) | qwen2.5:32b | 19GB | Best-in-class structured JSON output among open models | <3s | ✅ Yes | None |
| 5e Rules Decisions | qwen2.5:32b | 19GB | Decides what check is needed (C++ does the math) | <2s | ✅ Yes | None |
| Companion Combat AI | llama3.1:8b | 4.9GB | Simpler task, fast responses needed during combat flow | <500ms | ✅ Yes | None |
| Quest Generation | qwen2.5:32b | 19GB | Creative generation + structured output, runs during transitions | <5s | ✅ Yes | None |
| Memory Summarization | llama3.1:8b | 4.9GB | Background task, doesn't need to be best, just fast | Background | ✅ Yes | None |

**No new models needed.** All game systems are covered by the two installed models.

## Custom Modelfile Configurations

### DM Brain Configuration (qwen2.5:32b)

Create a custom Modelfile for the DM brain with optimized parameters:

```bash
# Save this as Modelfile-dm-brain
FROM qwen2.5:32b

PARAMETER temperature 0.7
PARAMETER top_p 0.9
PARAMETER num_predict 1024
PARAMETER num_ctx 8192
PARAMETER stop "```"
PARAMETER stop "Player:"
PARAMETER stop "USER:"
PARAMETER stop "SYSTEM:"
```

Create the custom model:
```bash
ollama create greymaw-dm -f Modelfile-dm-brain
```

### Companion Combat Configuration (llama3.1:8b)

Create a custom Modelfile for fast companion decisions:

```bash
# Save this as Modelfile-companion
FROM llama3.1:8b

PARAMETER temperature 0.5
PARAMETER top_p 0.9
PARAMETER num_predict 256
PARAMETER num_ctx 2048
PARAMETER stop "```"
```

Create the custom model:
```bash
ollama create greymaw-companion -f Modelfile-companion
```

### Memory Summarizer Configuration (llama3.1:8b)

```bash
# Save this as Modelfile-memory
FROM llama3.1:8b

PARAMETER temperature 0.3
PARAMETER num_predict 512
PARAMETER num_ctx 4096
```

Create the custom model:
```bash
ollama create greymaw-memory -f Modelfile-memory
```

## Setup Commands (Run in Order)

```bash
# 1. Verify Ollama is running
ollama list

# 2. Verify required base models are installed
# You should see qwen2.5:32b and llama3.1:8b in the list
# If missing, run:
# ollama pull qwen2.5:32b
# ollama pull llama3.1:8b

# 3. Create custom model configurations
# Save each Modelfile content above to a file, then:

echo 'FROM qwen2.5:32b
PARAMETER temperature 0.7
PARAMETER top_p 0.9
PARAMETER num_predict 1024
PARAMETER num_ctx 8192
PARAMETER stop "```"
PARAMETER stop "Player:"
PARAMETER stop "USER:"
PARAMETER stop "SYSTEM:"' > Modelfile-dm-brain

echo 'FROM llama3.1:8b
PARAMETER temperature 0.5
PARAMETER top_p 0.9
PARAMETER num_predict 256
PARAMETER num_ctx 2048
PARAMETER stop "```"' > Modelfile-companion

echo 'FROM llama3.1:8b
PARAMETER temperature 0.3
PARAMETER num_predict 512
PARAMETER num_ctx 4096' > Modelfile-memory

ollama create greymaw-dm -f Modelfile-dm-brain
ollama create greymaw-companion -f Modelfile-companion
ollama create greymaw-memory -f Modelfile-memory

# 4. Verify all models are available
ollama list
# Should show: qwen2.5:32b, llama3.1:8b, greymaw-dm, greymaw-companion, greymaw-memory
```

## Verification Test Script

Run these curl commands to verify each model works correctly before starting development:

### Test 1: DM Brain (greymaw-dm / qwen2.5:32b)

```bash
curl -s http://localhost:11434/api/generate -d '{
  "model": "greymaw-dm",
  "prompt": "You are a D&D Dungeon Master. The player enters a tavern. Describe the scene in 2-3 sentences. Respond ONLY with a JSON object: {\"narration\": \"your description\"}",
  "stream": false,
  "format": "json"
}' | python -m json.tool
```

**Expected:** A valid JSON response with a "narration" field containing a tavern description.
**Latency target:** < 5 seconds for first response (cold start may be slower).

### Test 2: Companion Combat (greymaw-companion / llama3.1:8b)

```bash
curl -s http://localhost:11434/api/generate -d '{
  "model": "greymaw-companion",
  "prompt": "You are Kael, a Fighter in combat. Enemies: 2 goblins (one wounded). Your HP: 28/28. Player HP: 20/24. Pick an action. Respond ONLY with JSON: {\"action\": \"attack\", \"target\": \"wounded goblin\", \"comment\": \"optional quip\"}",
  "stream": false,
  "format": "json"
}' | python -m json.tool
```

**Expected:** A valid JSON response with action, target, and optional comment.
**Latency target:** < 1 second.

### Test 3: Memory Summarizer (greymaw-memory / llama3.1:8b)

```bash
curl -s http://localhost:11434/api/generate -d '{
  "model": "greymaw-memory",
  "prompt": "Summarize this D&D session in 2-3 sentences: The player entered the Thornhaven tavern, spoke with Marta the innkeeper about missing villagers, recruited Kael the fighter as a companion, and purchased healing potions from the village merchant before heading toward the Greymaw Caverns.",
  "stream": false
}' | python -m json.tool
```

**Expected:** A concise summary of the session events.
**Latency target:** < 2 seconds.

### Test 4: JSON Format Reliability

```bash
curl -s http://localhost:11434/api/generate -d '{
  "model": "greymaw-dm",
  "prompt": "A player says: I try to persuade the guard to let us through. You are a DM. Respond with this exact JSON format and nothing else: {\"narration\": \"description\", \"check_required\": {\"type\": \"persuasion\", \"ability\": \"charisma\", \"dc\": 15, \"advantage\": false, \"disadvantage\": false}, \"on_success\": {\"narration\": \"success text\", \"actions\": [], \"world_changes\": []}, \"on_failure\": {\"narration\": \"failure text\", \"actions\": [], \"world_changes\": []}, \"companion_reaction\": null, \"scene_change\": null}",
  "stream": false,
  "format": "json"
}' | python -m json.tool
```

**Expected:** Valid JSON matching the schema. This tests the model's ability to produce the full DM response format.
**Latency target:** < 5 seconds.

### Test 5: Ollama Health Check

```bash
curl -s http://localhost:11434/api/tags | python -m json.tool
```

**Expected:** JSON listing all available models. This is the check the game performs on startup.

## VRAM & Performance Notes

- **qwen2.5:32b (19GB):** Will NOT fully fit in 16GB VRAM. Ollama automatically offloads layers to system RAM (you have 64GB, plenty of room). Expect slightly slower inference vs pure GPU but still workable for 2-3 second responses.
- **llama3.1:8b (4.9GB):** Fits entirely in VRAM. Extremely fast inference (~100-500ms).
- **Running both simultaneously:** Ollama can keep multiple models loaded. Set `OLLAMA_NUM_PARALLEL=2` environment variable. qwen2.5 will use VRAM + RAM, llama3.1 stays in VRAM. Test this configuration early (Milestone 2) to verify no issues.
- **VRAM budget:** Game rendering will use ~6-10GB VRAM depending on quality preset. qwen2.5 will primarily use system RAM for inference. llama3.1 at 4.9GB fits in remaining VRAM. Monitor with nvidia-smi during development.

## Troubleshooting

| Issue | Solution |
|---|---|
| Ollama not responding | Verify Ollama service is running: `ollama serve` or check system tray |
| Model not found | Run `ollama list` and verify model names. Pull if missing: `ollama pull qwen2.5:32b` |
| Very slow responses (>10s) | Check VRAM usage with `nvidia-smi`. Close other GPU applications. Reduce num_ctx. |
| Invalid JSON responses | Ensure `"format": "json"` is set in the request. Check the system prompt reinforces JSON-only output. |
| Out of memory | Only one large model (qwen2.5) should run at a time for VRAM-heavy operations. Reduce num_ctx from 8192 to 4096 if needed. |
| Game + Ollama VRAM conflict | Lower game graphics preset. qwen2.5 should offload to system RAM automatically. |
