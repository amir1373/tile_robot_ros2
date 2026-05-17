#!/usr/bin/env python3
"""Check that expected ROS 2 package files exist in this workspace."""

from __future__ import annotations

from pathlib import Path

EXPECTED = [
    "src/database/package.xml",
    "src/database/src/database_service.cpp",
    "src/database/src/postgres.cpp",
    "src/database_interfaces/package.xml",
]


def main() -> int:
    root = Path(__file__).resolve().parents[3]
    missing = [path for path in EXPECTED if not (root / path).exists()]

    if missing:
        print("Missing expected files:")
        for path in missing:
            print(f"  - {path}")
        return 1

    print("ROS 2 package layout looks complete.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())