# Follow-up from the editor changes review

Review of the recent sidebar, file-watching, LSP logging, and saving changes.
These are follow-up items, not claims that autosave is already implemented.
Existing tests passed (CTest 2/2 in `build/cmake`), but they do not cover all
of the scenarios below.

## Before enabling autosave

1. **Failed Save As leaves a file attached to the buffer.**
   `Files::save()` assigns a `File` before trying to write
   (`src/core/files.cpp:130–142`). If writing fails, the buffer still has a
   backing file and a subsequent Save As cannot follow the unnamed-buffer
   path. Keep it unnamed on failure, or explicitly support retrying with a new
   path. Test a failing write followed by a successful retry.

2. **Language-server logs steal editor focus.**
   LSP stderr is routed to `LogType::Error` (`src/plugin/lsp/lspplugin.cpp`),
   but the main-window log subscriber calls `showConsole()` for each message
   (`src/views/mainwindow.cpp:101–116`), which focuses the console. Make the
   messages available in the existing console without taking keyboard focus
   away from the editor or sidebar. Test repeated stderr messages while typing.

3. **Autosave and early-save triggers are not implemented.**
   The current work checks for disk conflicts but does not schedule a save
   after inactivity or save when leaving a file/view or losing window focus.
   Implement those only after the failure and conflict paths are sound;
   ensure timers act on the GUI thread and do not save unnamed or temporary
   interaction buffers. Terminal window-focus notifications may be unavailable.

## Performance and conflict correctness

4. **The watcher traverses build directories even though the project file
   list excludes them.** `src/linux/inotify.cpp:142–184` recursively adds
   watches without the CMake build-directory exclusion used by
   `src/files/project.cpp`. Large build trees can make startup expensive or
   exhaust available watches. Design and measure an exclusion/recovery policy
   before changing this performance-sensitive path. **Obtain explicit operator
   approval before changing performance-sensitive code** (`AGENTS.md`).

5. **Modification times are only an approximate conflict check.**
   `src/files/file.cpp:26–62` compares timestamps, and the check and write are
   not atomic. Preserved timestamps, coarse timestamp resolution, or another
   write between check and save can evade detection. This is the agreed
   lightweight initial approach, not a guarantee against overwriting a
   concurrent writer. Revisit stronger detection when warranted; also see
   `todo.md`. Do not add whole-file reads on every save without explicit
   performance approval.

No fixes from this review have been applied here.
