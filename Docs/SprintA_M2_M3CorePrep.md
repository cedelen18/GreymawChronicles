# Sprint A Implementation Note (Milestone 2 + Milestone 3 Core Prep)

## Implemented

### Milestone 2 (2.1-2.4)
- Added `UOllamaSubsystem` with async HTTP integration (`/api/tags`, `/api/generate`) using `FHttpModule`.
- Added startup health-check path on subsystem initialize.
- Added model inventory parsing and required-model validation for:
  - `qwen2.5:32b`
  - `llama3.1:8b`
- Added clear remediation output with exact commands when models are missing:
  - `ollama pull qwen2.5:32b`
  - `ollama pull llama3.1:8b`
- Added request timeout handling (15s), invalid response handling, retry support, and logging.
- Added `UOllamaRequest` wrapper for lifecycle + retry metadata.
- Added `UOllamaRequestQueue` with priority scheduling and per-model in-flight restriction.

### Milestone 3 Core Prep (3.3, 3.4, 3.5 + integration skeleton)
- Added `UDMPromptBuilder` with:
  - `BuildDMPrompt()` (7-section assembly pattern)
  - `BuildCompanionCombatPrompt()`
- Added `UDMResponseParser` with:
  - JSON repair helpers (trailing comma cleanup, JSON fence trimming)
  - Basic schema extraction to `FDMResponse`
  - Action whitelist validation hooks
- Added `UDMConversationHistory` with:
  - Circular buffer behavior (cap 15)
  - Prompt serialization (`ToPromptString`)
  - JSON serialize/deserialize round-trip functions
- Added compile-ready integration points in `UDMBrainSubsystem`:
  - prompt build -> ollama request -> parse -> delegate broadcast
  - conversation history update per exchange

### Build/Module
- Added/updated `GreymawChronicles.Build.cs` dependencies for HTTP/Json/UMG/Slate/SQLite stack.

### Tests (practical)
- Added automation tests for:
  - Conversation history capacity enforcement
  - JSON repair + parse basic happy path

## Next Steps
- Wire `OnOllamaHealthChanged` into concrete startup UI widget flow (blocking gate before main menu).
- Expand parser to full output schema fields (world changes, check branches, scene changes).
- Add richer priority queue preemption telemetry and stronger retry/backoff policy.
- Add full UE runtime validation playtest in-editor.
