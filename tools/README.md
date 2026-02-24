# Tools

This folder contains standalone helper scripts used during debug/validation of generated AIE control-code artifacts.

## Scripts

### Data_Integrity_Validator.py

Validates consistency between `UC_DMA_BD` references and the corresponding data payload labels/entries in a generated `.asm` file.

What it checks (high-level):
- Every `UC_DMA_BD ... @LABEL, SIZE, ...` references a label that exists in the data section.
- The referenced label’s payload length (number of `.long` entries) matches the `SIZE` field.

Usage:
- Validate a single file:
	- `python tools/Data_Integrity_Validator.py <input.asm>`
	- `python tools/Data_Integrity_Validator.py <input.asm> <output_report.txt>`
- Validate all `.asm` files in a directory (recursive):
	- `python tools/Data_Integrity_Validator.py <folder_path>`
	- `python tools/Data_Integrity_Validator.py <folder_path> <output_report_folder>`

Example:
- `python tools/Data_Integrity_Validator.py aie_runtime_control0.asm`
- `python tools/Data_Integrity_Validator.py .`
- `python tools/Data_Integrity_Validator.py . integrity_reports/`

Notes:
- The validator is intended to be run on generated assembly to catch missing/mismatched data labels early.
- When validating a directory and providing an output report folder, reports are written under that folder mirroring the input directory structure, with filenames ending in `.integrity_report.txt`.
- This README is expected to grow as more scripts are added under `tools/`.
