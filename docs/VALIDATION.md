# Validation record

Date: 2026-11-13

Passed in this Linux environment:

- Existing core tests compiled with g++ using C++11, -Wall, -Wextra and -pedantic.
- Weighted averages, grade boundaries, input validation, duplicate handling, SQLite save/update/search, rollback, restart persistence and course removal passed.
- Code::Blocks project XML parsed and paths updated for src/ and include/.
- Original GUI, model and database source bytes preserved exactly after relocation.
- Source archive excludes build products, databases and CSV exports.

Not executed here:

- Windows compiler and PowerShell build/release scripts (tools unavailable).
- Windows GUI layout, screen clearing, keyboard navigation, CSV dialog and portable second-PC launch.

Complete WINDOWS_CHECKLIST.md on Windows before sharing a runnable release. This deliverable is a source package with local release preparation scripts, not a prebuilt executable.
