# Binary release proposal — verification pending

Repository owner approved by TJay: `Parajose`.
Source repository: `Parajose/umat-cwa-studio`.
Visibility: private.
Proposed tag: `v0.1.0`.
Proposed release title: `UMaT CWA Studio 0.1.0 — Friend Preview`.

Source upload approved by TJay. A prebuilt Windows executable is not included.

## Draft release description

An early Windows desktop preview for entering student course results and viewing credit-weighted averages in a saved SQLite database.

- Object-oriented C++ classes for students, courses, storage and application screens.
- Navy-and-gold GUI, course input clearing, and a final saved-results table.
- Student search, course editing, semester averages and CSV export.
- Local data per Windows user; no server or account required.

Download the portable ZIP matching your Windows architecture, extract it, and open `UMaT_CWA_Studio.exe`. Start with fictional data. See START_HERE.txt for a worked example and feedback instructions.

Known limitations: fixed-size interface, unsigned executable, local unencrypted data, and no repeat-course policy. This is an independent educational project.

## Local preparation and verification

1. Build and package on Windows using `scripts/release_windows.ps1`.
2. Complete `docs/WINDOWS_CHECKLIST.md`, including a second-PC check.
3. Record Windows version, compiler version and manual-check outcome here before publication.
4. Attach the generated portable ZIP and its SHA-256 file; GitHub can supply source archives after a tag exists.

Current evidence: portable core tests can be run on Linux; Windows compilation, GUI interaction and portable runtime remain to be verified. Do not label this Windows-tested until those checks pass.

## Future binary release

After Windows verification, prepare the version tag and portable release assets for review. Choose a license with TJay before granting open-source reuse rights. No publication commands run automatically.
