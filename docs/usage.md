# Usage

## Choosing GUI or terminal mode

On Linux, `medit` starts in the terminal when both `DISPLAY` and
`WAYLAND_DISPLAY` are unset or empty, as in a normal SSH session. If either
variable is nonempty, the GUI remains the default, including SSH sessions with
X11 forwarding. Detection uses these environment variables; it does not probe
whether the display connection is working.

Use `--cli` or `--gui` to explicitly choose a mode. Explicit remote/client/server
modes are unchanged. Other platforms keep their existing GUI default.

For terminal mode, use a UTF-8 locale (check with `locale charmap`).

## Autosave

Edited files that already have a filename are saved after two seconds without
further edits, including files open in the background. Switching files or
leaving a text editor saves its changes sooner; when the GUI window loses focus,
all modified disk-backed buffers are saved. Unnamed buffers and
temporary console/interaction buffers are not saved automatically. Autosave
runs on the editor thread; diagnostics and redraws do not postpone it.

If the file's modification time differs from the version last read or saved,
autosave leaves both versions alone and reports the conflict in the existing
console. A manual save uses the existing overwrite confirmation. Modification
times are a lightweight check, not a guarantee against simultaneous writes.
Terminal window focus is not always reported, so `--cli` still relies on
editor/view changes and the inactivity timer. Saving a file does not dismiss a
disk conflict or ask for confirmation automatically.

## Project file panel

The panel is hidden initially. F3 shows and focuses it, or hides it; F6 moves
focus between the panel and the last text editor. Escape returns to the editor without hiding the
panel. Window shortcuts such as Ctrl+P (command palette) still work in the panel.

- Up/down or j/k select rows; left/right or h/l collapse/expand directories or
  move to their parent/first child. Home/End and Page Up/Down navigate the list.
- Enter toggles a directory or opens a file in the last active text-editor split.
  Clicking a directory toggles it; clicking a file opens it.
- `▸`/`▾` mark directories, `›` marks the selected row, and `●` marks loaded files.
- Following the active text file is enabled initially. Use **toggle follow active
  file** in the command palette to turn it off/on. Following reveals registered
  files without changing keyboard focus or repeatedly resetting tree navigation.

The tree uses the shared project's registered files, filtering known file types
and skipping directories named `build` (including nested ones) and CMake build
directories. Empty directories and files outside the
project root are not displayed. Configurable ignore patterns and Git-status
filtering are not implemented. Linux watches the project recursively, including
new and moved directories. Changes update the tree automatically; showing the
panel also refreshes the snapshot. If watches fail, a warning is logged and the
watcher retries and requests recovery snapshots instead of silently going stale.
Short-lived child directories (for example CMake's `TryCompile` directories) may
vanish during traversal without triggering a warning or periodic recovery. A
missing project root or a genuine watch/access failure still triggers recovery.

UTF-8 single-column filenames are displayed directly. Characters that do not fit
one cell (including double-width/combining characters), controls, and invalid
UTF-8 bytes are escaped in labels to keep the current screen buffers aligned.
Opening a file always uses its original, unchanged path.
