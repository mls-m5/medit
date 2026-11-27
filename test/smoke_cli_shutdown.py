#!/usr/bin/env python3
"""PTY regression: clean CLI exit restores the alternate screen and termios.

Run with: python3 test/smoke_cli_shutdown.py build/cmake/medit
Requires a UTF-8 locale, ncurses, and a usable xterm-256color terminfo entry.
"""

import os
import pathlib
import pty
import select
import signal
import struct
import subprocess
import sys
import tempfile
import termios
import time
import fcntl


def check(binary):
    with tempfile.TemporaryDirectory(prefix="medit-shutdown-") as home:
        file = pathlib.Path(home) / "note.md"
        file.write_text("shutdown test\n")
        master, slave = pty.openpty()
        fcntl.ioctl(master, termios.TIOCSWINSZ, struct.pack("HHHH", 30, 100, 0, 0))
        original = termios.tcgetattr(slave)
        env = dict(os.environ, HOME=home, TERM="xterm-256color", LC_ALL="C.UTF-8")
        env.pop("DISPLAY", None)
        env.pop("WAYLAND_DISPLAY", None)
        process = subprocess.Popen(
            [binary, "--cli", str(file)],
            stdin=slave,
            stdout=slave,
            stderr=slave,
            env=env,
            start_new_session=True,
        )
        output = bytearray()
        try:
            # Wait for the first frame rather than relying on a fixed startup delay.
            deadline = time.monotonic() + 5
            while time.monotonic() < deadline and b"shutdown test" not in output:
                ready, _, _ = select.select([master], [], [], 0.05)
                if ready:
                    try:
                        output.extend(os.read(master, 65536))
                    except OSError:
                        break
                if process.poll() is not None:
                    break
            if process.poll() is None:
                os.write(master, b"\x10quit\n")  # Ctrl+P, command palette.
            deadline = time.monotonic() + 5
            while time.monotonic() < deadline and process.poll() is None:
                ready, _, _ = select.select([master], [], [], 0.05)
                if ready:
                    try:
                        output.extend(os.read(master, 65536))
                    except OSError:
                        break
            assert process.poll() == 0, f"medit exited with {process.poll()}"
            assert b"\x1b[?1049l" in output, "alternate screen was not left"
            assert b"\x1bc" not in output, "terminal was reset after endwin"
            assert termios.tcgetattr(slave) == original, "terminal attributes were not restored"
        finally:
            if process.poll() is None:
                os.killpg(process.pid, signal.SIGKILL)
            process.wait()
            os.close(master)
            os.close(slave)


if __name__ == "__main__":
    binary = str(pathlib.Path(sys.argv[1]).resolve())
    for run in range(3):
        check(binary)
        print(f"CLI shutdown {run + 1}: OK")
