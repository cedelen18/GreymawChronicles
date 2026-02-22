# Milestone 6 Go/No-Go Checklist (Talking Demo)

**Milestone:** 6 - The Talking Demo  
**Scope:** UI ↔ DM brain end-to-end + first playable text interaction  
**Owner:** ____________________  
**Date:** ____________________  
**Build/Commit Under Test:** ____________________

## Gate Result

- **Overall Decision:** `PENDING` (`GO` | `NO-GO`)
- **Blocking Issues:** ____________________
- **Notes:** ____________________

---

## Exit Criteria (Must Pass for GO)

| ID | Criterion | Pass Criteria | Current Status | Evidence / Notes |
|---|---|---|---|---|
| M6-01 | UI input routes to DM brain | Player input from UI triggers `ProcessPlayerInput` and receives a DM narration response in the DM panel | PENDING | |
| M6-02 | Tavern conversation turn works | Input: `I walk up to the bar and talk to the innkeeper` produces coherent tavern-context narration | PENDING | |
| M6-03 | In-character response works | Input: `I ask about the disappearances` yields an in-character response as Marta (or configured innkeeper NPC) | PENDING | |
| M6-04 | Check-required flow works | Input: `I try to arm wrestle Kael` produces `check_required` behavior and rules/check handling path executes (or logs expected placeholder) | PENDING | |
| M6-05 | Fallback path works | Malformed or invalid model response does not crash. User sees graceful fallback narration/error handling | PENDING | |
| M6-06 | Conversation history continuity | Multi-turn context persists; later DM output reflects prior turns | PENDING | |
| M6-07 | HUD stats visible | Player + companion HP/stat strip visible during test | PENDING | |
| M6-08 | Ollama readiness | Ollama health check succeeds and required model(s) are present before play test | PENDING | |
| M6-09 | No crash / hard lock | No crash, assert, or unrecoverable freeze during all smoke scenarios | PENDING | |
| M6-10 | Windows UE build green | Unreal Windows Development Editor build succeeds for current commit | PENDING | |

---

## Suggested Test Inputs

1. `I walk up to the bar and talk to the innkeeper`
2. `I ask about the disappearances`
3. `I try to arm wrestle Kael`
4. Fallback trigger input (or malformed JSON simulation) per current implementation

---

## Defect Log (If Any)

| Sev | ID/Title | Repro | Owner | Status |
|---|---|---|---|---|
| | | | | |

---

## Sign-off

- **QA:** ____________________
- **Engineering:** ____________________
- **Producer/Owner:** ____________________
