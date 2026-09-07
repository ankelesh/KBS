"""
Zips Content/Fab (the downloaded FAB assets, kept out of git) into a single
7z archive for personal backup.

Usage: python zip_fab.py <output_path.7z> [--7z-path C:\\path\\to\\7z.exe]
"""
import argparse
import shutil
import subprocess
import sys
from pathlib import Path

CONTENT_DIR = Path(__file__).resolve().parent
FAB_DIR = CONTENT_DIR / "Fab"


def find_7z(explicit_path):
    if explicit_path:
        return explicit_path
    found = shutil.which("7z")
    if found:
        return found
    for candidate in (
        r"C:\Program Files\7-Zip\7z.exe",
        r"C:\Program Files (x86)\7-Zip\7z.exe",
    ):
        if Path(candidate).exists():
            return candidate
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path, help="Path to write the .7z archive to")
    parser.add_argument("--7z-path", dest="sevenzip_path", default=None,
                         help="Explicit path to 7z.exe if it isn't on PATH")
    args = parser.parse_args()

    if not FAB_DIR.is_dir():
        print(f"error: {FAB_DIR} does not exist", file=sys.stderr)
        sys.exit(1)

    sevenzip = find_7z(args.sevenzip_path)
    if not sevenzip:
        print("error: could not find 7z on PATH or in common install locations. "
              "Pass --7z-path explicitly.", file=sys.stderr)
        sys.exit(1)

    output = args.output
    output.parent.mkdir(parents=True, exist_ok=True)

    cmd = [sevenzip, "a", "-t7z", str(output), str(FAB_DIR)]
    print("running:", " ".join(cmd))
    result = subprocess.run(cmd)
    sys.exit(result.returncode)


if __name__ == "__main__":
    main()
