#!/usr/bin/env python3
"""
CS2 Offset Extractor
Extracts needed offsets from cs2-dumper output and creates a single JSON file
"""

import json
import os
import sys
from pathlib import Path

# Define the offsets we need
NEEDED_OFFSETS = {
    "client_dll": {
        # Client.dll base offsets (from offsets.json)
        "offsets": [
            "dwEntityList",
            "dwViewMatrix",
            "dwLocalPlayerController",
            "dwGlobalVars",
            "dwPlantedC4",
            "dwViewAngles",
            "dwLocalPlayerPawn",
        ],
        # Pawn offsets (from client_dll.json -> classes)
        "classes": {
            "CCSPlayerController": [
                "m_iPing",
                "m_hPawn",
                "m_steamID",
                "m_iszPlayerName",
                "m_bIsLocalPlayerController",
                "m_pInGameMoneyServices",
            ],
            "CCSPlayerController_InGameMoneyServices": ["m_iAccount"],
            "C_BaseEntity": [
                "m_iHealth",
                "m_iTeamNum",
                "m_vOldOrigin",
                "m_pGameSceneNode",
            ],
            "C_CSPlayerPawn": [
                "m_bIsScoped",
                "m_iShotsFired",
                "m_ArmorValue",
                "m_bIsDefusing",
                "m_pClippingWeapon",
                "m_entitySpottedState",
                "m_flFlashOverlayAlpha",
                "m_aimPunchAngle",
                "m_iIDEntIndex",
                "m_flFlashDuration",
                "m_aimPunchCache",
                "m_vecLastClipCameraPos",
                "m_angEyeAngles",
                "m_fFlags",
                "m_vecViewOffset",
            ],
            "EntitySpottedState_t": ["m_bSpottedByMask"],
            "C_PlantedC4": ["m_bC4Activated", "m_nBombSite"],
            "CGameSceneNode": ["m_vecAbsOrigin"],
        },
    }
}


def load_json(filepath):
    """Load JSON file"""
    try:
        with open(filepath, "r", encoding="utf-8") as f:
            return json.load(f)
    except Exception as e:
        print(f"Error loading {filepath}: {e}")
        return None


def extract_class_field(client_data, class_name, field_name):
    """Extract a specific field from a class"""
    try:
        classes = client_data.get("client.dll", {}).get("classes", {})
        class_data = classes.get(class_name, {})
        fields = class_data.get("fields", {})
        return fields.get(field_name, None)
    except Exception as e:
        print(f"Warning: Failed to extract {class_name}.{field_name}: {e}")
        return None


def main():
    # Get paths
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    output_dir = project_root / "output"
    github_dir = project_root / ".github"

    # Create .github directory if it doesn't exist
    github_dir.mkdir(exist_ok=True)

    print("CS2 Offset Extractor")
    print("=" * 50)
    print(f"Output dir: {output_dir}")
    print(f"GitHub dir: {github_dir}")

    # Load source files
    offsets_file = output_dir / "offsets.json"
    client_file = output_dir / "client_dll.json"

    if not offsets_file.exists():
        print(f"Error: {offsets_file} not found!")
        print("Please run cs2-dumper first to generate offset files.")
        sys.exit(1)

    if not client_file.exists():
        print(f"Error: {client_file} not found!")
        print("Please run cs2-dumper first to generate offset files.")
        sys.exit(1)

    print(f"\nLoading {offsets_file.name}...")
    offsets_data = load_json(offsets_file)

    print(f"Loading {client_file.name}...")
    client_data = load_json(client_file)

    if not offsets_data or not client_data:
        print("Error: Failed to load offset files!")
        sys.exit(1)

    # Extract offsets
    result = {
        "version": "1.0",
        "timestamp": None,  # Will be set by dumper
        "client_dll": {"offsets": {}, "classes": {}},
    }

    print("\nExtracting offsets...")

    # Extract base offsets from offsets.json
    client_offsets = offsets_data.get("client.dll", {})
    for offset_name in NEEDED_OFFSETS["client_dll"]["offsets"]:
        value = client_offsets.get(offset_name)
        if value is not None:
            result["client_dll"]["offsets"][offset_name] = value
            print(f"  [OK] {offset_name}: 0x{value:X}")
        else:
            print(f"  [FAIL] {offset_name}: NOT FOUND")

    # Extract class field offsets from client_dll.json
    print("\nExtracting class fields...")
    for class_name, fields in NEEDED_OFFSETS["client_dll"]["classes"].items():
        result["client_dll"]["classes"][class_name] = {}
        print(f"\n  {class_name}:")

        for field_name in fields:
            value = extract_class_field(client_data, class_name, field_name)
            if value is not None:
                result["client_dll"]["classes"][class_name][field_name] = value
                print(f"    [OK] {field_name}: 0x{value:X}")
            else:
                print(f"    [FAIL] {field_name}: NOT FOUND")

    # Save to .github/offsets.json
    output_file = github_dir / "offsets.json"
    print(f"\nSaving to {output_file}...")

    with open(output_file, "w", encoding="utf-8") as f:
        json.dump(result, f, indent=2)

    print(f"[OK] Successfully saved offsets to {output_file}")
    print("\nDone!")

    # Print summary
    total_offsets = len(result["client_dll"]["offsets"])
    total_fields = sum(
        len(fields) for fields in result["client_dll"]["classes"].values()
    )
    print(f"\nSummary:")
    print(f"  Base offsets: {total_offsets}")
    print(f"  Class fields: {total_fields}")
    print(f"  Total: {total_offsets + total_fields}")


if __name__ == "__main__":
    main()
