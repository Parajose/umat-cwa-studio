# Release process

Repository: [Parajose/umat-cwa-studio](https://github.com/Parajose/umat-cwa-studio) — public.

## Automated build

The Windows workflow compiles the app, runs model/SQLite tests, exercises the actual GUI with isolated synthetic data, and packages a portable ZIP plus its SHA-256 checksum. Its artifact also includes the saved-results screenshot.

## Publish a tested version

1. Update `VERSION`, `CHANGELOG.md` and `docs/RELEASE_NOTES.md` for the intended version.
2. Open Actions → Windows build and test → Run workflow.
3. Select the main branch and enable the publish checkbox.
4. The workflow builds and tests first. Only a successful run publishes the versioned preview release, portable ZIP, checksum and screenshot.
5. Record the successful run in `VALIDATION.md` and complete remaining manual checks when testing new Windows/display scenarios.

The local packaging script never publishes. The manual release workflow uses GitHub's repository-scoped token. Do not embed personal access tokens or student data in source or assets. Choose a licensing policy with the author before adding open-source reuse rights.
