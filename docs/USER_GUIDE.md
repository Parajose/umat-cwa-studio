# UMaT CWA Studio

A C++11 object-oriented Windows desktop application with a navy-and-gold interface and a real SQLite database. This is an independent educational project, not an official UMaT application.

## Open and run in Code::Blocks

1. Extract this ZIP into a normal folder on a Windows 10/11 computer.
2. Open **UMaT_CWA_Studio.cbp** in Code::Blocks with the MinGW C++ compiler installed.
3. Choose **Debug** or **Release** from the build-target menu.
4. Press **F9** to build and run.

Open the included project rather than pasting src/main.cpp into the old console project: this project links the Windows GUI libraries and uses the Windows subsystem, so no console opens.

Alternatively, with MinGW's `g++` available in PATH, run `build_windows.bat` from the project root.

No Qt, MySQL server, or downloaded SQLite DLL is required by this implementation. It loads the SQLite DLL from the Windows system directory. If that component is unavailable, the app displays an error and does not start. Do not copy arbitrary system DLLs into your project.

## How to use it

1. Click **+ New student**. Enter the ID and name, then click **Continue to courses**. The student-entry screen disappears.
2. Enter a course code, title, semester label, final numerical mark out of 100, and credit hours.
3. Click **Add course +**. The code, title, mark and credit fields clear for the next course; the semester label stays to reduce retyping. The course appears in the draft table.
4. Repeat for the other courses. Use consistent semester labels, e.g. `2026/27 - Semester 1`.
5. Click **Save & View Database**. The entry screen disappears, a transaction saves the whole record, and the saved database screen opens with the student selected.
6. Select any student to inspect their courses, credits, letter grades, weighted marks and semester averages.
7. Search by name or ID. Click Search to apply it; clear the field and click Search to show all students.
8. Select a student and choose **Edit selected** to append courses or remove an incorrect course and re-enter it. Changes reach the database only when you save. Student identity fields are fixed in this version.
9. **Export all CSV** exports every saved student's course results, even if a search is active.
10. **Open data folder** reveals the actual SQLite database file.

Unsaved changes prompt before navigating away or closing. A partially typed course must be added or cleared before the record can be saved. This prevents accidentally losing it.

## Where is the database?

The app creates `umat_cwa.db` in:

    %LOCALAPPDATA%\UMaTCWAStudio\

On a typical PC this is `C:\Users\YourName\AppData\Local\UMaTCWAStudio\umat_cwa.db`.

**Open data folder** takes you directly there. The database is local to that Windows user, persists between runs and is shared by copies of this app running under that user. It is not online or synchronised between friends.

The built-in database screen is a viewer of the real saved records, not a console printout. For raw tables, open the file with an SQLite database viewer. The two tables are `students` and `courses`. Close the app before backing up or manually changing the database. Existing text-file records from the earlier console application are not imported.

## Calculation and academic scope

    Weighted mark = final numerical mark * course credits
    Average = sum of weighted marks / sum of credits

- Enter ALL completed, applicable courses to get a CWA.
- One semester only gives that semester's SWA.
- Semester averages use only courses matching that semester label.
- The app labels the overall result “Average” because it cannot know whether you entered your complete history.
- Marks: A >= 80, B >= 70, C >= 60, D >= 50, F < 50.
- Calculations retain double precision; displayed results use two decimal places.
- Credits may be fractional and must be > 0 and <= 100. The upper bound is an application limit, not a university regulation.
- One result per course code per student is supported. This version does not implement repeat/replacement, incomplete, audited-course or graduation-classification policies. It does not decide whether a student has satisfied graduation requirements.

Example: marks 80, 70 and 90 with credits 3, 3 and 2 produce weighted marks 630, credits 8 and average **78.75**. Add a fourth course with mark 60 and 4 credits in another semester: overall average becomes **72.50**, while the first semester stays **78.75** and the new semester is **60.00**.

Formula and grading reference: UMaT, Conduct of Undergraduate Programmes (2022), sections 6.1 and 6.3:
https://umat.edu.gh/images/Students%20Resources/2023/Conduct%20of%20undergraduate%20programmes%202022.pdf

## OOP structure

- `Course`: private course data, validation, grade and weighted-mark methods.
- `Student`: private student data; owns Course objects; calculates totals and averages.
- `Database`: owns the database connection; saves and retrieves Student objects; RAII closes connections and finalizes statements.
- `Application`: owns the window and screens; handles user interaction.

Objects combine data and behaviour. Composition connects Student to its courses and Application to its database. No inheritance is forced into the design where it is unnecessary. The Win32 callback forwards events into the Application object.

Database writes use bound parameters and an explicit transaction. Duplicate IDs and course codes are rejected. Validation occurs before saving. Technical references:
https://www.sqlite.org/c3ref/bind_blob.html
https://www.sqlite.org/lang_transaction.html

## Share with friends

Build the **Release** target, then send `bin/Release/UMaT_CWA_Studio.exe` in a ZIP. Have your friend extract it before running. Each Windows user gets their own empty database on first launch. Do not include your database unless you intend to share the records.

The Release build statically links the MinGW runtimes to reduce runtime dependencies. Any other missing runtime dependency depends on the installed MinGW distribution. This package contains source code and build configuration, not a precompiled EXE.

## Validation and limitations

The portable model and actual SQLite storage layer were compiled using g++ with C++11 and warnings enabled. Automated checks cover weighted calculations, grade boundaries, invalid marks/credits, duplicate records, search, save/update, transactions, course removal and persistence after reopening.

**The Windows GUI has not been compiled or run in this Linux environment.** Build and manually check it on Windows before distributing. Included checks: `WINDOWS_CHECKLIST.md`.

The window is fixed-size, designed for a standard desktop at 100% scaling. Very small displays or high display scaling may need layout adjustments. All records are local and unencrypted. No login, networking or multi-user permissions are implemented.

## Optional core tests

Linux with g++ and the SQLite runtime:

    g++ -std=c++11 -Wall -Wextra -pedantic tests/core_tests.cpp -ldl -o core_tests
    ./core_tests

On Windows with MinGW (using the system SQLite DLL):

    g++ -std=c++11 tests/core_tests.cpp -o core_tests.exe
    core_tests.exe

Run tests from a writable folder. Test records are synthetic and removed afterwards.
