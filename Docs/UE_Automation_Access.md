# UE Automation Access Configuration

Enabled automation and editor scripting plugins for Greymaw Chronicles (UE 5.7).
These support both interactive editor workflows and headless CI/agent automation.

## Plugin Enablement Status

| Plugin | Enabled | Category | Purpose |
|--------|---------|----------|---------|
| SQLiteCore | Yes (pre-existing) | Runtime | Persistent memory, session storage |
| PythonScriptPlugin | Yes | Experimental | Python editor scripting, automation test authoring |
| EditorScriptingUtilities | Yes | Editor | Blueprint/C++ editor automation helpers |
| RemoteControl | Yes | VirtualProduction | HTTP/WebSocket API for external tool integration |
| RemoteControlWebInterface | Yes | VirtualProduction | Web UI for Remote Control presets |
| Blutility | N/A | Engine core | Editor Utility Widgets built into UE 5.x (no plugin needed) |
| GameplayDebugger | N/A | Engine core | Built into engine in UE 5.x (no plugin needed) |

## Configuration Defaults

### Remote Control API
- **HTTP port**: 30010 (localhost only)
- **WebSocket port**: 30020 (localhost only)
- **Bind address**: `127.0.0.1` (loopback only, never exposed to network)
- Config location: `Config/DefaultEngine.ini` under `[/Script/RemoteControl.RemoteControlSettings]`

### Python Script Plugin
- **Developer mode**: enabled (allows running arbitrary Python in editor console)
- **Remote execution**: disabled (no external Python IDE bridge in committed config)
- Config location: `Config/DefaultEngine.ini` under `[/Script/PythonScriptPlugin.PythonScriptPluginSettings]`

## Local-Only Steps (Not in Committed Config)

These must be done manually in the editor on each developer machine:

1. **Remote Control Web Interface**: After first launch, open `http://127.0.0.1:30010` in a browser to confirm the web API is responding. This only works when the editor is running.

2. **Python Script startup scripts**: Place `.py` files in `Content/Python/` to auto-run on editor startup. These are gitignored by default (Content/ is in .gitignore for binary assets).

3. **Editor Utility Widgets (Blutility)**: Create new Editor Utility Widgets via Content Browser > Add > Editor Utilities > Editor Utility Widget. No plugin enablement needed.

4. **Gameplay Debugger**: Press `'` (apostrophe) in PIE to toggle the gameplay debugger overlay. No plugin enablement needed; it is compiled into the engine.

## Security Notes

- Remote Control API is bound to `127.0.0.1` only. **Never change this to `0.0.0.0`** in committed config for a public repo.
- No authentication tokens, API keys, or network-facing endpoints are configured.
- Python remote execution is disabled to prevent accidental exposure of an IDE bridge port.
- All automation access is editor-only and does not ship in packaged builds.

## Use Cases

- **Claude/Codex CI**: Automation tests run via `UnrealEditor-Cmd.exe` with `-NullRHI` already work. Python scripting adds the ability to write test orchestration scripts.
- **Editor scripting**: `EditorScriptingUtilities` provides `UEditorAssetLibrary`, `UEditorLevelLibrary`, etc. for batch asset operations.
- **Remote tool integration**: External tools (OpenClaw, custom dashboards) can query/control the editor via the Remote Control HTTP API at `localhost:30010`.
