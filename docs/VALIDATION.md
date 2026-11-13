# Validation record

Date: 2026-11-13

## Automated Windows verification

[Initial successful Windows run](https://github.com/Parajose/umat-cwa-studio/actions/runs/43498049361).

Environment: GitHub-hosted Windows Server 2025 x64; MSYS2 UCRT64 GCC 16.2.0. The same Win32 desktop executable is packaged for Windows 10/11 x64.

Passed:

- Windows GUI compilation and static MinGW runtime linking.
- Calculation, grade boundaries, invalid input, duplicate handling, SQLite save/update/search, rollback, restart persistence and course removal.
- Actual GUI process launch, switching from student details to course entry, and clearing course inputs after Add.
- Saving replaces entry with the two database-style tables.
- Synthetic 80/70/90 marks with 3/3/2 credits display 78.75% and persist after closing and reopening the app.
- ZIP packaging and SHA-256 creation; no student database or CSV exports are included.

The screenshot uses fictional test records. The GUI test runs with `--data-dir` in a temporary folder and removes it after completion.

The first screenshot revealed default window placement overlapping the taskbar on a small display. The window now centres inside the desktop work area; follow-up CI checks verify the current source before release publication.

## Additional checks

Portable model and SQLite regression tests also pass on Linux with C++11 and warnings enabled. The original course-clearing and database-view behaviour remains in place.

## Practical limits

Automated smoke checks do not certify every Windows 10/11 version, display scale, keyboard path, save dialog or CSV spreadsheet import. Continue `WINDOWS_CHECKLIST.md` for broader manual testing. The program is an unsigned educational preview with local unencrypted data and no repeat-course/graduation policy.
