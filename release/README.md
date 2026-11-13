# Portable release structure

The source ZIP does not contain an EXE. On Windows, run `scripts/release_windows.ps1` to generate:

```text
dist/
  UMaT-CWA-Studio-0.1.0-windows-x64.zip
  UMaT-CWA-Studio-0.1.0-windows-x64.zip.sha256
```

The ZIP has one top-level folder containing only:

- `UMaT_CWA_Studio.exe`
- `START_HERE.txt`
- `USER_GUIDE.md`
- `CHANGELOG.md`
- `BUILD_INFO.txt`

An x86 compiler substitutes `x86` for `x64`. No student database or exported records are packaged. The script stages only these files; complete the Windows manual checks before sharing.
