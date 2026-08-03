#!/usr/bin/env python3
"""
Twister serial PTY bridge for Realtek Ameba boards.

Bridges a local or remote serial port to a local PTY for twister DeviceHandler.
Invoked automatically via the serial_pty field; can also be run manually for
debugging.

Local serial:
  python twister_serial_pty.py --port /dev/ttyUSB0 [--baud 1500000] [--reset]

Remote serial:
  python twister_serial_pty.py --port COM5 --remote-server 172.29.35.108 \
      [--baud 1500000] [--reset]
"""

import argparse
import logging
import os
import sys
import threading
import time
from pathlib import Path

# Locate RemoteSerial: search upward from this script's location, matching
# the same discovery logic used in tools/meta_tools/scripts/monitor/base/serial_reader.py.
_here = Path(__file__).resolve().parent
_possible_paths = [
    _here.parent.parent.parent.parent.parent / "tools" / "ameba" / "RemoteService",
    *(_p / "tools" / "ameba" / "RemoteService" for _p in _here.parents),
]
_remote_service_path = next((p for p in _possible_paths if p.exists()), None)

RemoteSerial = None
if _remote_service_path:
    sys.path.insert(0, str(_remote_service_path))
    try:
        from remote_serial import RemoteSerial
    except (ImportError, Exception):
        RemoteSerial = None

REMOTE_PORT = 58916

logging.basicConfig(level=logging.WARNING)


def parse_args():
    parser = argparse.ArgumentParser(
        description="Twister serial PTY bridge for Realtek Ameba boards"
    )
    parser.add_argument("--port", required=True,
                        help="Serial port: local e.g. /dev/ttyUSB0, remote e.g. COM5")
    parser.add_argument("--remote-server", default=None,
                        help="Remote serial server IP (omit for local serial port)")
    parser.add_argument("--baud", type=int, default=1500000,
                        help="Baud rate (default 1500000)")
    parser.add_argument("--reset", action="store_true",
                        help="Reset device after connecting")
    return parser.parse_args()


def make_serial(args):
    """Return a serial object (local or remote) with a unified interface."""
    if args.remote_server:
        if RemoteSerial is None:
            sys.stderr.write(
                f"[twister_serial_pty] RemoteSerial not found"
                + (f" at {_remote_service_path}" if _remote_service_path else "") + ".\n"
                "Make sure the ameba_tools repo is checked out under tools/ameba/.\n"
            )
            sys.exit(1)
        logger = logging.getLogger("remote_serial")
        logger.setLevel(logging.WARNING)
        return RemoteSerial(
            logger=logger,
            remote_server=args.remote_server,
            remote_port=REMOTE_PORT,
            port=args.port,
            baudrate=args.baud,
            source="monitor",
        )

    # Local serial via pyserial
    try:
        import serial as pyserial
    except ImportError:
        sys.stderr.write("[twister_serial_pty] pyserial not found. "
                         "Install with: pip install pyserial\n")
        sys.exit(1)

    class _LocalSerial:
        """Thin pyserial wrapper that matches the RemoteSerial interface."""
        def __init__(self, port, baud):
            self._port = port
            self._baud = baud
            self._ser = None

        def open(self):
            self._ser = pyserial.Serial(
                port=self._port,
                baudrate=self._baud,
                timeout=0,
                parity=pyserial.PARITY_NONE,
                stopbits=pyserial.STOPBITS_ONE,
                bytesize=pyserial.EIGHTBITS,
            )

        def close(self):
            if self._ser and self._ser.isOpen():
                self._ser.close()

        def inWaiting(self):
            return self._ser.in_waiting if self._ser else 0

        def read(self, size=1):
            return self._ser.read(size)

        def write(self, data):
            self._ser.write(data)

        def flushInput(self):
            if self._ser:
                self._ser.reset_input_buffer()

        def reset_device(self):
            """Hardware reset via RTS/DTR (standard Ameba reset sequence)."""
            if not self._ser:
                return
            self._ser.dtr = False
            self._ser.rts = True
            time.sleep(0.2)
            self._ser.rts = False
            self._ser.dtr = False

    return _LocalSerial(args.port, args.baud)


def bridge(ser, stop_event):
    """Read from serial and write to stdout (twister's pty end)."""
    while not stop_event.is_set():
        try:
            waiting = ser.inWaiting()
        except Exception:
            break
        if waiting > 0:
            try:
                chunk = ser.read(min(waiting, 4096))
            except Exception:
                break
            if chunk:
                try:
                    os.write(sys.stdout.fileno(), chunk)
                except OSError:
                    break
        else:
            time.sleep(0.005)


def main():
    args = parse_args()
    ser = make_serial(args)

    try:
        ser.open()
    except Exception as e:
        sys.stderr.write(f"[twister_serial_pty] Failed to open serial: {e}\n")
        sys.exit(1)

    stop_event = threading.Event()

    # Start reader before reset so no output is lost
    reader = threading.Thread(target=bridge, args=(ser, stop_event), daemon=True)
    reader.start()

    if args.reset:
        ser.flushInput()
        ser.reset_device()

    # Main thread: stdin → serial (twister rarely writes, but keep the channel open)
    try:
        while True:
            try:
                data = os.read(sys.stdin.fileno(), 256)
                if not data:
                    break
                ser.write(data)
            except OSError:
                break
    except KeyboardInterrupt:
        pass
    finally:
        stop_event.set()
        try:
            ser.close()
        except Exception:
            pass


if __name__ == "__main__":
    main()
