param(
    [string]$OllamaBaseUrl = "http://localhost:11434",
    [string]$RequiredModels = "llama3.1:8b",
    [switch]$SkipInGameSteps
)

$ErrorActionPreference = "Stop"

function Write-Section($text) {
    Write-Host "`n=== $text ===" -ForegroundColor Cyan
}

function Pass($msg) { Write-Host "[PASS] $msg" -ForegroundColor Green }
function Fail($msg) { Write-Host "[FAIL] $msg" -ForegroundColor Red }
function Warn($msg) { Write-Host "[WARN] $msg" -ForegroundColor Yellow }

Write-Section "Milestone 6 Manual Smoke Test"
Write-Host "Ollama Base URL: $OllamaBaseUrl"
Write-Host "Required Models: $RequiredModels"

$models = $RequiredModels.Split(",") | ForEach-Object { $_.Trim() } | Where-Object { $_ -ne "" }

# 1) Ollama health
Write-Section "1) Ollama health"
try {
    $tags = Invoke-RestMethod -Method Get -Uri "$OllamaBaseUrl/api/tags" -TimeoutSec 10
    Pass "Ollama reachable at $OllamaBaseUrl"
} catch {
    Fail "Could not reach Ollama at $OllamaBaseUrl"
    Write-Host $_.Exception.Message
    exit 1
}

# 2) Required model presence
Write-Section "2) Required model presence"
$available = @()
if ($tags.models) {
    $available = $tags.models | ForEach-Object { $_.name }
}

$missing = @()
foreach ($m in $models) {
    if ($available -contains $m) {
        Pass "Model present: $m"
    } else {
        $missing += $m
        Fail "Model missing: $m"
    }
}

if ($missing.Count -gt 0) {
    Warn "Install missing models, e.g.:"
    $missing | ForEach-Object { Write-Host "  ollama pull $_" }
}

if ($SkipInGameSteps) {
    Write-Section "Skipped in-game steps"
    Write-Host "Run without -SkipInGameSteps to print full checklist."
    exit 0
}

# 3-5) Manual in-game checklist
Write-Section "3) One DM turn (manual)"
Write-Host "Input: I walk up to the bar and talk to the innkeeper"
Write-Host "Expect: DM narration appears in UI; no errors/crash."
Write-Host "Result: [ ] PASS  [ ] FAIL    Notes: ____________________"

Write-Section "4) One check-required turn (manual)"
Write-Host "Input: I try to arm wrestle Kael"
Write-Host "Expect: check_required flow/log appears; check handling executes gracefully."
Write-Host "Result: [ ] PASS  [ ] FAIL    Notes: ____________________"

Write-Section "5) One fallback turn (manual)"
Write-Host "Input: Trigger malformed/invalid response case per current debug path"
Write-Host "Expect: graceful fallback response; no crash/assert."
Write-Host "Result: [ ] PASS  [ ] FAIL    Notes: ____________________"

Write-Section "Done"
Write-Host "Record final GO/NO-GO in Docs/Milestone6_GoNoGo_Checklist.md"
