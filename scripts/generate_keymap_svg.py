#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import logging
import os
import subprocess
import sys
from pathlib import Path
from typing import Any

REPO_ROOT = Path(__file__).parent.parent.resolve()
sys.path.insert(0, str(REPO_ROOT / "qmk_firmware" / "lib" / "python"))
os.environ.setdefault("ORIG_CWD", str(REPO_ROOT))

os.chdir(REPO_ROOT / "qmk_firmware")

from qmk.commands import parse_configurator_json
from qmk.keymap import (
    c2json,
    list_keymaps,
    locate_keymap,
)

QMK_FIRMWARE = REPO_ROOT / "qmk_firmware"


DEFAULT_OUTPUT = str(REPO_ROOT / "assets")


class KeymapError(Exception):
    """Base exception for keymap operations."""
    pass


class KeymapNotFoundError(KeymapError):
    """Raised when a keymap file cannot be found."""
    pass


class KeymapParseError(KeymapError):
    """Raised when keymap parsing fails."""
    pass


class SVGGenerationError(KeymapError):
    """Raised when SVG generation fails."""
    pass


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate SVG keymaps for QMK keyboards",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "-o", "--output",
        default=os.environ.get("OUTPUT_DIR", DEFAULT_OUTPUT),
        help="Output directory (default: ./assets or OUTPUT_DIR env var)"
    )
    parser.add_argument(
        "-c", "--columns",
        type=int,
        default=int(os.environ.get("COLUMNS", "10")),
        help="Column count for parsing (default: 10 or COLUMNS env var)"
    )
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Enable verbose (debug) logging"
    )
    parser.add_argument(
        "keyboard",
        help="KEYBOARD or KEYBOARD:KEYMAP"
    )
    return parser.parse_args()


def setup_logging(verbose: bool) -> None:
    env_level = os.environ.get("LOG_LEVEL", "").upper()
    
    if env_level:
        level = getattr(logging, env_level, None)
        if not isinstance(level, int):
            level = logging.WARNING
    elif verbose:
        level = logging.DEBUG
    else:
        level = logging.WARNING
    
    logging.basicConfig(
        level=level,
        format="%(levelname)s: %(message)s"
    )


def keyboard_to_filename(kb: str) -> str:
    return kb.replace("/", "_")


def generate_keymap_json(kb: str, km: str, keymap_file: Path) -> dict[str, Any]:
    logging.debug(f"Generating keymap JSON for {kb}:{km} from {keymap_file}")
    if keymap_file.suffix == ".c":
        return c2json(kb, km, keymap_file, use_cpp=False)
    else:
        return parse_configurator_json(keymap_file)


def generate_svg(kb: str, km: str, keymap_json: dict[str, Any], output_dir: str, columns: int) -> Path:
    filename = f"{keyboard_to_filename(kb)}_{km}.svg"
    output_path = Path(output_dir) / filename
    
    logging.debug(f"Generating SVG for {kb}:{km}, output: {output_path}")
    
    try:
        output_path.parent.mkdir(parents=True, exist_ok=True)
    except OSError as e:
        raise KeymapError(f"Failed to create output directory {output_dir}: {e}")

    json_str = json.dumps(keymap_json)

    try:
        parse_proc = subprocess.Popen(
            [
                "keymap", "parse",
                "--qmk-keymap-json", "-",
                "--columns", str(columns),
                "--output", "-",
            ],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
    except FileNotFoundError as e:
        raise KeymapError(
            f"Failed to run keymap parse command: 'keymap' not found. "
            f"Ensure the keymap CLI tool is installed: {e}"
        )
    except PermissionError as e:
        raise KeymapError(f"Permission denied running keymap parse: {e}")

    parsed, parse_err = parse_proc.communicate(input=json_str)

    if parse_proc.returncode != 0:
        raise KeymapParseError(f"keymap parse failed (exit {parse_proc.returncode}): {parse_err}")

    try:
        draw_proc = subprocess.Popen(
            ["keymap", "draw", "--qmk-keyboard", kb, "-"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
    except FileNotFoundError as e:
        raise KeymapError(
            f"Failed to run keymap draw command: 'keymap' not found. "
            f"Ensure the keymap CLI tool is installed: {e}"
        )
    except PermissionError as e:
        raise KeymapError(f"Permission denied running keymap draw: {e}")

    svg, draw_err = draw_proc.communicate(input=parsed)

    if draw_proc.returncode != 0:
        raise SVGGenerationError(f"keymap draw failed (exit {draw_proc.returncode}): {draw_err}")

    try:
        output_path.write_text(svg)
    except OSError as e:
        raise KeymapError(f"Failed to write SVG to {output_path}: {e}")

    logging.debug(f"Wrote SVG to {output_path}")
    return output_path


def process_keymap(kb: str, km: str, output_dir: str, columns: int) -> Path:
    logging.debug(f"Processing keymap {kb}:{km}")
    keymap_file = locate_keymap(kb, km)
    
    if not keymap_file:
        raise KeymapNotFoundError(f"No keymap file found for {kb}:{km}")

    logging.debug(f"Keymap file: {keymap_file}")

    keymap_json = generate_keymap_json(kb, km, keymap_file)
    return generate_svg(kb, km, keymap_json, output_dir, columns)


def main() -> None:
    args = parse_args()
    setup_logging(args.verbose)

    if ":" in args.keyboard:
        kb, km = args.keyboard.split(":", 1)
    else:
        kb = args.keyboard
        km = "all"

    logging.info(f"Processing keymaps for keyboard: {kb}")

    if km == "all" or not km:
        try:
            keymaps = list_keymaps(kb)
        except Exception as e:
            logging.error(f"Failed to list keymaps for {kb}: {e}")
            sys.exit(1)
        
        if not keymaps:
            logging.error(f"No keymaps found for keyboard {kb}")
            sys.exit(1)
            
        logging.info(f"Found {len(keymaps)} keymaps to process")
        
        failed = False
        for k in keymaps:
            try:
                output_path = process_keymap(kb, k, args.output, args.columns)
                print(output_path)
            except KeymapError as e:
                logging.warning(str(e))
                failed = True
        
        if failed:
            sys.exit(1)
    else:
        logging.info(f"Processing single keymap: {km}")
        try:
            output_path = process_keymap(kb, km, args.output, args.columns)
            print(output_path)
        except KeymapError as e:
            logging.error(str(e))
            sys.exit(1)


if __name__ == "__main__":
    main()
