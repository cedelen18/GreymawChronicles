# Character Sheet Data Assets Scaffolding (Task 3.2)

Binary `.uasset` `UGCCharacterSheet` Data Assets cannot be reliably generated headless from this environment.

## What is provided
- 13 JSON seed files (`*.seed.json`) matching `UGCCharacterSheet::FromJSON` compact schema.
- Deterministic naming for eventual UE assets.

## Required editor conversion (exact steps)
1. Open project in Unreal Editor.
2. In Content Browser, create folder: `Content/Data/CharacterSheets/` (if missing).
3. For each `*.seed.json` file below:
   - Right-click → **Miscellaneous → Data Asset**
   - Pick class: `GCCharacterSheet`.
   - Name asset exactly as the seed basename without `.seed` (example: `DA_GC_Human_Fighter`).
4. Open each Data Asset and paste/apply values from corresponding JSON.
   - Optional: add a utility BP/C++ importer to call `FromJSON` per asset automatically.
5. Save all assets.
6. Validate with PIE/log that all 13 assets load.

## Seed files
- DA_GC_Human_Fighter.seed.json
- DA_GC_Human_Rogue.seed.json
- DA_GC_Human_Wizard.seed.json
- DA_GC_Human_Cleric.seed.json
- DA_GC_Elf_Fighter.seed.json
- DA_GC_Elf_Rogue.seed.json
- DA_GC_Elf_Wizard.seed.json
- DA_GC_Elf_Cleric.seed.json
- DA_GC_Dwarf_Fighter.seed.json
- DA_GC_Dwarf_Rogue.seed.json
- DA_GC_Dwarf_Wizard.seed.json
- DA_GC_Dwarf_Cleric.seed.json
- DA_GC_Kael_Companion.seed.json

## Notes
- These are scaffolds for Milestone 3.2; tune exact stats/features/spells against GDD Section 2/4 in editor.
- Kael seed is included and should be adjusted to match final narrative sheet exactly.
