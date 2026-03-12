# CS2 Offset Update System

This directory contains tools to automatically fetch and update CS2 game offsets.

## Files

- **`extract_offsets.py`** - Python script that extracts needed offsets from cs2-dumper output
- **`update_offsets.bat`** - Batch file that orchestrates the entire update process

## Quick Start

Simply run the batch file:

```bash
update_offsets.bat
```

This will:
1. Download cs2-dumper (if not already downloaded)
2. Run cs2-dumper to extract offsets from CS2
3. Parse the output and create `.github/offsets.json`

## Requirements

- **Python 3.x** - Must be installed and in PATH
- **CS2 running** - The game must be running when you execute the dumper
- **Internet connection** - To download cs2-dumper on first run

## Workflow

### 1. Update Offsets Locally

```bash
# Run the update script
update_offsets.bat

# This creates/updates:
#   - tmp/cs2-dumper/cs2-dumper.exe
#   - output/*.json (raw cs2-dumper output)
#   - .github/offsets.json (extracted offsets)
```

### 2. Commit to GitHub

```bash
git add .github/offsets.json
git commit -m "Update CS2 offsets"
git push origin main
```

### 3. HTTP Updater

The application's HTTP updater (`src/updater/Updater.cpp`) will fetch offsets from:

```
https://raw.githubusercontent.com/zlc1004/cs2-external-esp/main/.github/offsets.json
```

## Output Format

The `.github/offsets.json` file contains:

```json
{
  "version": "1.0",
  "timestamp": null,
  "client_dll": {
    "offsets": {
      "dwViewAngles": 36804168,
      "dwEntityList": 38462056,
      ...
    },
    "classes": {
      "C_CSPlayerPawn": {
        "m_iHealth": 852,
        "m_vecViewOffset": 3416,
        ...
      }
    }
  }
}
```

## Needed Offsets

The system automatically extracts these offsets:

### Base Offsets (client.dll)
- dwEntityList
- dwViewMatrix
- dwLocalPlayerController
- dwGlobalVars
- dwPlantedC4
- dwViewAngles
- dwLocalPlayerPawn

### Class Field Offsets
- **CCSPlayerController**: m_iPing, m_hPawn, m_steamID, etc.
- **C_BaseEntity**: m_iHealth, m_iTeamNum, m_pGameSceneNode
- **C_CSPlayerPawn**: m_bIsScoped, m_iShotsFired, m_vecViewOffset, etc.
- **EntitySpottedState_t**: m_bSpottedByMask
- **C_PlantedC4**: m_bC4Activated, m_nBombSite
- **CGameSceneNode**: m_vecAbsOrigin

## Troubleshooting

### "CS2 is not running"
- Start CS2 before running the batch file
- You can continue anyway, but cs2-dumper may fail

### "Python not found"
- Install Python 3 from https://www.python.org/downloads/
- Make sure to check "Add Python to PATH" during installation

### "Failed to download cs2-dumper"
- Check your internet connection
- Manually download from: https://github.com/a2x/cs2-dumper/releases
- Place `cs2-dumper.exe` in `tmp/cs2-dumper/` folder

### "Some offsets NOT FOUND"
- This is normal - some offsets may have changed class names in CS2 updates
- The script will extract all available offsets
- Check the console output to see which offsets were found

## Manual Updates

If you need to add more offsets:

1. Edit `scripts/extract_offsets.py`
2. Add to the `NEEDED_OFFSETS` dictionary
3. Run `update_offsets.bat` again

## Credits

- **cs2-dumper** by a2x: https://github.com/a2x/cs2-dumper
- Offset extraction script by zlc1004
