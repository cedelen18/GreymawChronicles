# Greymaw Chronicles

An AI-driven tabletop RPG experience built with Unreal Engine 5.7 and local
large language models. A living D&D 5e session where an AI Dungeon Master
narrates the adventure while a real-time 3D world serves as the theater of
the mind.

## Prerequisites

- **Unreal Engine 5.7.x** (installed via Epic Games Launcher)
- **Visual Studio 2022 or later** with the C++ game development workload
- **Windows 10/11** (64-bit)
- **[Ollama](https://ollama.com/)** for local LLM inference
- **Git LFS** (`git lfs install` before cloning)

## Getting Started

```bash
git lfs install
git clone https://github.com/cedelen18/GreymawChronicles.git
```

1. Open `GreymawChronicles.uproject` in Unreal Editor 5.7.
2. Install the required Ollama models (see `Docs/GreymawChronicles_OllamaSetup.md`).
3. Build and play from the editor (Alt+P), or use the command line:

```
"<YOUR_UE_PATH>\Engine\Build\BatchFiles\Build.bat" ^
  GreymawChronicles Win64 Development ^
  -Project="<FULL_PATH>\GreymawChronicles.uproject" -WaitMutex
```

## Running Tests

Automated tests use UE's built-in automation framework and run headless:

```
"<YOUR_UE_PATH>\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" ^
  "<FULL_PATH>\GreymawChronicles.uproject" ^
  -ExecCmds="Automation RunTests Greymaw" -Unattended -NullRHI -Log
```

Test suites cover DM subsystem logic, rules engine (dice, combat, spells),
intent classification, and milestone smoke checks.

## CI/CD

This project uses GitHub Actions with two workflows:

| Workflow | Runner | Triggers | Purpose |
|----------|--------|----------|---------|
| **UE5 Build & Test** | Self-hosted Windows | PR to main, nightly, manual | Full compile + automation tests |
| **Repository Hygiene** | Hosted ubuntu-latest | Every push, PR to main | Lint, docs freshness, repo validation |

### Self-Hosted Runner Setup

The UE5 build workflow requires a self-hosted Windows runner with:

- Unreal Engine 5.7 installed
- Visual Studio Build Tools (C++ workload)
- Git LFS support

Before your first run, update the placeholder values in
`.github/workflows/ue5-build.yml`:

| Placeholder | Replace With |
|-------------|-------------|
| `YOUR_RUNNER_LABEL` | Your GitHub Actions runner label |
| `YOUR_UE_PATH` | Absolute path to your UE 5.7 install root |

See [GitHub's self-hosted runner docs](https://docs.github.com/en/actions/hosting-your-own-runners)
for installation instructions.

## Project Structure

```
Config/             Engine and project configuration
Content/            UE assets, blueprints, maps, audio, UI
Docs/               Design documents, setup guides, model configs
Scripts/            Build and test utility scripts
Source/             C++ game module
  GreymawChronicles/
    Private/        Implementation (.cpp)
      ActionSystem/ Action director and cinematic camera
      Core/         Game instance and mode
      DungeonMaster/ AI brain, intent classification, narration
      Gameplay/     Dice, ability checks, character sheets
      Ollama/       Local LLM HTTP integration
      Rules/        D&D 5e combat and spell resolution
      Scene/        Level and world state management
      Tests/        Automation test suites
      UI/           HUD widgets, input, debug overlay
    Public/         Headers (.h) — mirrors Private structure
```

## Security Practices

- **No secrets in source.** Configuration files must not contain tokens,
  API keys, or credentials. Use environment variables or local config
  overrides for sensitive values.
- **Secret scanning.** Enable
  [GitHub secret scanning](https://docs.github.com/en/code-security/secret-scanning)
  and push protection on your fork.
- **History hygiene.** If secrets are accidentally committed, use
  [git-filter-repo](https://github.com/newren/git-filter-repo) or
  [BFG Repo-Cleaner](https://rtyley.github.io/bfg-repo-cleaner/) to purge
  them from history, then rotate the compromised credentials.
- **Self-hosted runners.** Keep runner machines on private networks. Never
  commit UE installation paths, IP addresses, or hostnames to workflow files.

## Contributing

1. Fork the repository.
2. Create a feature branch from `master`.
3. Ensure the Repository Hygiene workflow passes (header guards, .gitignore,
   required files).
4. Open a pull request with a clear description of changes.
5. Wait for CI checks before requesting review.

## License

License information to be added.
