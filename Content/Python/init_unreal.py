"""
Greymaw Chronicles — Editor Startup Script

Runs automatically when the UE Editor opens (requires PythonScriptPlugin).
Add to Project Settings → Plugins → Python → Startup Scripts: init_unreal.py

Use this for:
  - Registering custom editor commands
  - Setting up automation helpers
  - Wiring up Remote Control presets
"""

import unreal


def _on_editor_ready():
    """Called once the editor is fully initialized."""
    unreal.log("GreymawChronicles: Python environment ready")


_on_editor_ready()
