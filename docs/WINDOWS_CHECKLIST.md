# Windows verification before sharing

- Open the CBP project in Code::Blocks with MinGW and build both Debug and Release.
- Confirm the app opens with no console and that text/buttons fit your display.
- Use Tab and Shift+Tab to move through fields; verify buttons work by keyboard and mouse.
- Add a synthetic student and the three sample courses from README; expect 78.75.
- Check that Continue replaces the student form, Add clears the course fields, and Save replaces the form with the database screen.
- Try an empty student name, non-numeric mark, mark 101, credits 0, duplicate course code and duplicate student ID. Expect validation without losing valid draft courses.
- Leave a partially typed course and click Save. Expect a prompt to add or clear it.
- Navigate away with unsaved input or close the app; cancel the discard prompt and confirm the draft remains.
- Save, close, reopen and check the student and courses persist.
- Search by ID and name; clear the search and confirm all records return.
- Select a student and choose Edit selected. Remove and replace a course, then save and reopen.
- Append a fourth course in another semester; verify overall 72.50, original SWA 78.75 and new SWA 60.00.
- Confirm Open data folder shows umat_cwa.db and CSV export contains saved rows.
- Test the Release executable outside Code::Blocks and on a second Windows PC before sharing it broadly.

- Run scripts/release_windows.ps1 and extract its dist/ ZIP; test that exact EXE.
- Confirm the ZIP has only the five files listed in release/README.md and no personal database or exports.
- Verify its SHA-256 against the supplied .sha256 file with Get-FileHash.
- Record Windows version, display scaling, compiler version and the test outcome in the draft release notes.
