#! /usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2024 Realtek Semiconductor Corp.
# SPDX-License-Identifier: Apache-2.0

import os
import sys
import argparse
import base64
import json
import subprocess
from pathlib import Path

FLASH_TOOL = str((Path.cwd() / 'tools' / 'meta_tools' / 'scripts' / 'flash' / 'AmebaFlash.py').resolve())

class MemoryInfo:
    MEMORY_TYPE_RAM = 0
    MEMORY_TYPE_NOR = 1
    MEMORY_TYPE_NAND = 2

profiles_path = (Path.cwd() / "tools" / "meta_tools" / "scripts" / "flash" / "Devices" / "Profiles").resolve()

profiles = {
    "amebag2": str((profiles_path / "AmebaGreen2_FreeRTOS_NOR.rdev").resolve()),
    "amebadplus": str((profiles_path / "AmebaDplus_FreeRTOS_NOR.rdev").resolve()),
}

def get_profile_path(device: str, check_exists: bool = False) -> str:
    """
    Return profile path for given device.

    Args:
        device: device name (case-insensitive).
        check_exists: when True, verify the file exists.

    Returns:
        Absolute path to the profile file.

    Raises:
        ValueError: if device is not supported or empty.
        FileNotFoundError: if check_exists is True and file does not exist.
    """
    if not device:
        raise ValueError("device must not be empty")

    key = device.lower()
    if key not in profiles:
        valid = ", ".join(sorted(profiles.keys()))
        raise ValueError(f"unsupported device: {device}. valid options: {valid}")

    path = profiles[key]
    if check_exists and not os.path.isfile(path):
        raise FileNotFoundError(f"profile file not found: {path}")

    return path

def ensure_tool_path_exists(tool_path: str) -> None:
    """
    Check if tool_path exists and is a directory.
    Raise FileNotFoundError if it does not exist.
    """
    if not tool_path:
        raise ValueError("tool_path must not be empty")

    if not os.path.exists(tool_path):
        raise FileNotFoundError(f"tool_path does not exist: {tool_path}")

def setup_parser(parser: argparse.ArgumentParser) -> argparse.ArgumentParser:
    parser.add_argument('--tool-path', help='Path to flash.py (absolute or relative)')
    parser.add_argument('--device', help='Device type')
    parser.add_argument('--image-dir', help="directory of image files")

    # Serial
    parser.add_argument('--port', action='append',
                        help='Serial port (repeatable), e.g. --port COM3 --port COM4')
    parser.add_argument('--baudrate', default='1500000', help='Serial baudrate (default: 1500000)')

    # Memory and addressing
    parser.add_argument('--memory-type', choices=['nor', 'nand', 'ram'], default='nor',
                        help='Memory type (default: nor)')
    parser.add_argument('--images', nargs=3, action='append', metavar=('image-name', 'start-address', 'end-address'),
                    help="user define image layout")

    # Erase control
    parser.add_argument('--chip-erase', action='store_true', help='Chip erase (NOR only)')

    # Logging and remote
    parser.add_argument('--log-level', default='info', help='Log level (info/debug/warn/error)')
    parser.add_argument('--log-file', help='Log file path')
    parser.add_argument('--remote-server', help='Remote serial server IP')
    parser.add_argument('--remote-password', help='Remote serial server password')

def main(args):
    if args is None:
        raise ValueError("Argument 'args' must not be None.")

    profile = get_profile_path(args.device)

    if not args.tool_path:
        args.tool_path = FLASH_TOOL

    ensure_tool_path_exists(args.tool_path)

    cmds = [sys.executable, args.tool_path, '--download', '--profile', profile]

    if not args.port:
        raise ValueError("Serial port is invalid")

    cmds.extend(['--port'])
    cmds.extend(args.port)
    cmds.extend(['--baudrate', args.baudrate])
    cmds.extend(['--memory-type', args.memory_type])
    cmds.extend(['--log-level', args.log_level])

    if args.log_file:
        cmds.extend(['--log-file', args.log_file])
    if args.remote_server:
        cmds.extend(['--remote-server', args.remote_server])
    if args.remote_password:
        cmds.extend(['--remote-password', str(args.remote_password)])

    if args.chip_erase:
        cmds.extend(['--chip-erase'])

    if not args.images:
        cmds.extend(['--image-dir', args.image_dir])
    else:
        partition_table = []

        if args.memory_type == "nand":
            memory_type = MemoryInfo.MEMORY_TYPE_NAND
        elif args.memory_type == "ram":
            memory_type = MemoryInfo.MEMORY_TYPE_RAM
        else:
            memory_type = MemoryInfo.MEMORY_TYPE_NOR

        # 1. Argparse.images format [[image-name, start-address, end-address], ...]
        for group in args.images:
            image_name_with_path = os.path.realpath(os.path.join(args.image_dir, group[0]))
            image_name = os.path.basename(image_name_with_path)
            try:
                start_addr = int(group[1], 16)
            except Exception as err:
                raise ValueError(f"Start addr in invalid: {err}")
                sys.exit(1)

            try:
                end_addr = int(group[2], 16)
            except Exception as err:
                raise ValueError(f"End addr in invalid: {err}")
                sys.exit(1)

            partition_table.append({
                "ImageName": image_name_with_path,
                "StartAddress": start_addr,
                "EndAddress": end_addr,
                "FullErase": False,
                "MemoryType": memory_type,
                "Mandatory": True,
                "Description": image_name
            })

        # 2. Convert list to json-str
        partition_table_json_string = json.dumps(partition_table)

        # 3. Encode json-str to bytes
        partition_table_bytes = partition_table_json_string.encode('utf-8')

        # 4. Base64 encode bytes
        partition_table_base64 = base64.b64encode(partition_table_bytes).decode('utf-8')

        cmds.extend(['--partition-table', partition_table_base64])

    subprocess.check_call(cmds)
