# UMaT CWA Studio

An object-oriented C++ Windows desktop application for entering student results, calculating credit-weighted averages, and viewing saved records in SQLite. Created by Joseph Tetteh (TJay). Independent educational project; not an official university application.

**Preview release: 0.1.0.** This download contains source and release preparation tools. It does not contain a precompiled Windows executable. Windows compilation and visual verification are still required before distributing a runnable build.

## Interface and behaviour

- Navy-and-gold graphical interface; no console window when running the app.
- Continue replaces the student-details screen with course entry.
- Add course clears course inputs while retaining the semester label.
- Save & View Database replaces entry with the saved student/course tables.
- Search, edit course results, semester summaries, persistent SQLite storage, and CSV export.

## Build and run on Windows

Requirements: Windows 10/11, a MinGW-w64 C++ compiler (`g++`) on PATH, and Windows PowerShell. A 64-bit compiler produces a build for 64-bit Windows. No Python, CMake, Qt, or database server is needed on Windows.

From the extracted project folder:

```powershell
.\build_windows.bat
.\bin\Release\UMaT_CWA_Studio.exe
```

The batch file returns a nonzero code on failure and leaves the compiler errors visible. If `g++` is unavailable, add the compiler's `bin` directory to PATH, then open a new terminal.

Alternatively open `UMaT_CWA_Studio.cbp` in Code::Blocks configured with MinGW, select **Release**, then press **F9**. Open the included project, rather than an old console project.

Direct compiler command (same source and libraries):

```powershell
g++ -std=c++11 -O2 -Wall -Wextra -Iinclude src/main.cpp -o bin/Release/UMaT_CWA_Studio.exe -mwindows -static -lcomctl32 -lshell32 -lole32 -lcomdlg32 -lgdi32 -luser32
```

Create `bin/Release` first when using the direct command. The static link flag includes the MinGW runtime libraries. The application still uses Windows system libraries, including `winsqlite3.dll`; never download replacement system DLLs from third-party sites.

## Prepare the ZIP for friends

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts\release_windows.ps1
```

This builds the app, compiles and runs the existing core tests, and creates a versioned ZIP plus SHA-256 file in `dist/`. It stages only the executable and friend-facing documentation; no student database, CSV export, source code, or credentials are included. Compiler architecture is recorded automatically. `ExecutionPolicy Bypass` applies only to this PowerShell process.

**Before sending:** complete [the Windows checklist](docs/WINDOWS_CHECKLIST.md), including running the staged app on a second PC without your compiler installed. The script cannot verify visual layout or interaction. It never uploads, pushes, tags, or publishes anything.

Friends extract the entire portable ZIP, open its folder, and double-click `UMaT_CWA_Studio.exe`. They do not need a compiler. See [START_HERE.txt](docs/START_HERE.txt).

## Data and calculations

Each Windows user gets their own database at `%LOCALAPPDATA%\\UMaTCWAStudio\\umat_cwa.db`. The records persist across launches and are not synchronised between friends. Existing copies run by the same Windows user share this location. Close the app before backing up the database. Records are local and unencrypted.

Weighted average = sum(mark × credits) / sum(credits). Enter all applicable completed courses for CWA, or one semester for SWA. The app calls the overall value “Average” because it cannot establish whether the history is complete. Example: marks 80, 70, 90 with credits 3, 3, 2 give **78.75**. Grading and repeat-course limitations are explained in the [user guide](docs/USER_GUIDE.md).

## Project layout

| Path | Purpose |
| --- | --- |
| `src/main.cpp` | Win32 Application class, screens and event handling |
| `include/Models.h` | Course and Student classes, validation and calculations |
| `include/Database.h` | SQLite connection and transactional storage with RAII |
| `tests/core_tests.cpp` | Existing model and SQLite regression checks |
| `scripts/` | Windows build and portable release preparation |
| `docs/` | User guide, manual checklist and draft GitHub release notes |
| `release/README.md` | Expected portable release contents |
| `UMaT_CWA_Studio.cbp` | Code::Blocks Debug/Release project |

## Run core tests

Windows:

```powershell
New-Item -ItemType Directory -Force build
g++ -std=c++11 -Wall -Wextra -pedantic tests/core_tests.cpp -o build/core_tests.exe
.\build\core_tests.exe
```

Linux (tests only; GUI is Windows-specific):

```sh
mkdir -p build
g++ -std=c++11 -Wall -Wextra -pedantic tests/core_tests.cpp -ldl -o build/core_tests
(cd build && ./core_tests)
```

Linux needs the SQLite runtime (`libsqlite3.so.0`). Tests use a temporary database named `cwa_test_temporary.db` in the working directory and remove it; run them in a disposable build folder. Keep assertions enabled (do not define `NDEBUG`).

## GitHub repository

Source published with TJay's approval at `Parajose/umat-cwa-studio` as a private repository. Windows executable builds and a tagged binary release remain pending verification.

[Release notes and verification steps](docs/GITHUB_RELEASE_DRAFT.md) are included for review. No open-source license has been selected; author approval is needed before adding a redistribution license.

## Known limits

The GUI is fixed at a 1010 × 680 client area and may need adjustments at high display scaling. There is no login, encryption, multi-user service, repeat-course policy, or import from the old text-file application. This package preserves the original GUI implementation; it does not claim Windows runtime validation in this Linux environment.
