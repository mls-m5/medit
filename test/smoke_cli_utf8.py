#!/usr/bin/env python3
"""PTY smoke test for UTF-8 ncurses input and output.

Run with: python3 test/smoke_cli_utf8.py build/utf8-review/medit
Requires a UTF-8 locale and xterm-256color terminfo.
"""

import curses
import fcntl
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


def collect(master, process, needle, timeout=5):
    output = bytearray()
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline and needle not in output:
        ready, _, _ = select.select([master], [], [], 0.05)
        if ready:
            try:
                output.extend(os.read(master, 65536))
            except OSError:
                break
        if process.poll() is not None:
            break
    return bytes(output)


def main(binary):
    with tempfile.TemporaryDirectory(prefix="medit-utf8-") as root:
        (pathlib.Path(root) / ".medit.json").write_text("{}")
        path = pathlib.Path(root) / "note.md"
        path.write_text("ASCII\n")
        master, slave = pty.openpty()
        fcntl.ioctl(slave, termios.TIOCSWINSZ, struct.pack("HHHH", 24, 80, 0, 0))
        env = dict(os.environ, HOME=root, TERM="xterm-256color", LC_ALL="C.UTF-8")
        process = subprocess.Popen(
            [str(binary), "--cli", str(path)],
            cwd=root,
            stdin=slave,
            stdout=slave,
            stderr=slave,
            env=env,
            start_new_session=True,
        )
        os.close(slave)
        try:
            assert b"ASCII" in collect(master, process, b"ASCII"), "no first frame"
            curses.setupterm("xterm-256color")
            os.write(master, curses.tigetstr("kf3"))
            panel = collect(master, process, b"note.md")
            assert b"Project" in panel and b"note.md" in panel, "broken tree frame"
            os.write(master, b"\x1b")  # Escape returns focus to the editor.
            # Normal mode -> insert mode, then one single- and one double-width char.
            os.write(master, "iå界".encode("utf-8"))
            output = collect(master, process, "å界".encode("utf-8"))
            assert "å界".encode("utf-8") in output, "UTF-8 input not rendered"
            assert process.poll() is None, "CLI exited unexpectedly"
        finally:
            if process.poll() is None:
                os.killpg(process.pid, signal.SIGTERM)
            try:
                process.wait(timeout=3)
            except subprocess.TimeoutExpired:
                os.killpg(process.pid, signal.SIGKILL)
                process.wait()
            os.close(master)


if __name__ == "__main__":
    main(pathlib.Path(sys.argv[1]).resolve())
    print("CLI UTF-8 input/output and project tree: OK")
