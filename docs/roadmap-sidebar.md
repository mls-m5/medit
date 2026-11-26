# Sidebar and project tree roadmap

## Goal and working approach

Start with a visible sidebar, then connect it to the project model and finally
make filesystem updates reliable. Keep each step small, independently testable,
and useful without completing the whole roadmap.

The sidebar manages window space and tool selection. A dedicated tree view
manages tree rendering and navigation. `Project` owns the project file list;
views must not maintain a separate filesystem scan or subscribe directly to
Linux inotify events.

Use the shared view and `IScreen` abstractions so GUI and CLI behave consistently.
Keep existing local changes and submodules untouched.

## 1. Visual sidebar prototype — first implementation

- [x] Add a separate sidebar view with dummy text, such as "Project".
- [x] Integrate it into `MainWindow::resize()` and `draw()`, outside `_editors`.
- [x] Reserve a fixed initial width and a separator; split the remaining width
      between the existing editors, including any remainder columns.
- [x] Keep the console and status bar full-width initially.
- [x] Clamp dimensions on narrow or short windows; avoid unsigned underflow.
      Temporarily hide the panel when there is insufficient editor space.
- [x] Register `toggle_sidebar` through `StaticCommandRegister`, making it
      available in the command palette. Use F3 as the initial shortcut.
- [x] Keep keyboard focus in the editor for this non-interactive prototype.

Acceptance: a sidebar can be shown and hidden in GUI and CLI without overlapping
editor content or breaking existing editor splits, console, or status display.
No project-model or filesystem-watcher changes are needed in this step.

Tests in this step:

- Layout tests for hidden/visible sidebar, multiple editors, console visibility,
  and very small window dimensions. Prefer testing a small layout calculation
  directly if constructing `MainWindow` requires unrelated runtime services.
- Rendering checks through the existing `test/mock/screen/mockscreen.h` where
  practical: dummy text and separator stay within their assigned region.
- Manual GUI and CLI checks: toggle via shortcut and command palette, resize,
  split editors, show the console, and check cursor/mouse coordinates. Include
  UTF-8 filenames or text with both single- and double-width characters.

Implementation notes and validation:

- The prototype starts hidden to preserve the usual editor layout. F3 toggles
  it on/off. It uses 24 content columns plus one separator and hides temporarily
  when it cannot leave at least eight columns per editor.
- `src/views/mainwindowlayout.h` keeps layout calculations independently testable;
  `src/views/sidebar.*` renders the non-interactive placeholder.
- Nine tests in `test/sidebar_test.cpp` cover geometry, console row allocation,
  tiny windows, rendering bounds, hidden/empty panels, and a wrapping-progress
  regression exposed by narrow editor splits.
- Release build and both CTest entries passed in `build/json-upgrade`.
- CLI pseudo-terminal smoke checks passed for startup with UTF-8 text (`åäö 界`),
  F3, the command-palette action, narrow/wide resize, two editor splits, the
  full-width console separator, and resizing down to 1x1 and back.
- Interactive GUI review is still pending (no display in the build environment).
  Precise cursor, clipping, and mouse behavior with double-width text still needs
  interactive verification; the smoke checks are not a substitute for that review.

## 2. Sidebar focus and window-level commands

- [x] Add `focus_sidebar` and `focus_editor`; use F6 to switch between the sidebar
      and the last active text editor.
- [x] Make Escape return from the sidebar to the editor without hiding it.
- [x] Add mouse focus handling for the sidebar.
- [x] Dispatch a small set of window-level commands independently of editor
      modes. Do not expose all text-editing commands to a non-editor tool.
- [x] Restore focus before executing a command-palette action, so an action that
      focuses the sidebar is not immediately overridden by the palette callback.
- [x] Preserve the last active text-editor target when the sidebar has focus.
      Define safe behavior for paste and other editor-only actions.

Tests: focus transitions, hiding a focused panel, Escape, mouse hit boundaries,
shortcuts from both regions, and focusing the sidebar through the command palette.

Implementation decisions:

- `MainWindow` owns a `KeyMap` for F3, F6, Ctrl+P, and Ctrl+T. A small factory in
  `mainwindow.cpp` defines the bindings. There is no default action: unmatched
  keys go to the focused view.
- ParentMode remains an ordinary editor parent mode with no boolean parameter.
  Its existing F3, Ctrl+P, and Ctrl+T bindings move to the window map; all other
  bindings stay there unchanged. No new Mode class is introduced.
- Ctrl+T still calls `showConsole()` directly. It opens/focuses the console rather
  than toggling visibility; console Escape behavior is unchanged. There is no
  new `show_console` command registration.
- Sidebar Escape is handled by the sidebar itself. Other unmatched tool input
  and paste are ignored rather than applied to the background text editor.
- F6 opens the sidebar if needed. Hiding it or resizing until it no longer fits
  returns focus to the last text editor; widening the window does not steal focus.
- Focused sidebar rendering highlights its title and shows "sidebar" in the
  status bar. Cancelling the command palette restores sidebar focus when it was
  opened there; executing a command first restores the editor so focus commands
  can choose their final target explicitly.

Validation: layout/rendering tests live in `test/sidebar_test.cpp`; focused-window
integration tests in `test/sidebarfocus_test.cpp` cover dispatch, editor fallback,
focus transitions, popup commands, mouse boundaries, and protection against
background edits. Release build and both CTest entries passed in
`build/json-upgrade`. CLI pseudo-terminal smoke checks also passed for F6,
Escape, F3, Ctrl+P focus/cancel, Ctrl+T, text-buffer protection, normal editing,
and narrow/wide resize using a file containing `åäö 界`.

GUI and precise double-width terminal-coordinate review remain manual checks.

## 3. Observable project file list

- [x] Add subscription and unsubscription for project file-list changes.
- [x] Notify after a cache refresh or a real membership change, on the GUI
      thread. File-content-only changes should not produce membership events.
- [x] Apply consistent file-selection rules to the initial scan and added paths.
- [x] Fix cache removal to erase the full removed range and safely handle absent
      paths. Ignore duplicate additions.
- [x] Define subscriber lifetime and queued-callback behavior explicitly.

Tests in `test/project_test.cpp`, using the existing directory-notification mock:
refresh, add, duplicate add, remove, absent remove, content-only change, filtering,
and unsubscribe. Drain the job queue deterministically; do not rely on sleeps.

Implementation notes:

- `subscribeToFilesChanged(callback, ref)` and
  `unsubscribeFromFilesChanged(ref)` follow the existing callback/reference
  pattern. They are GUI-thread operations; subscribing does not invoke a callback.
- A callback observes the updated, sorted, unique list of normalized absolute
  paths and updated extension counts. Refreshes with unchanged membership do not
  emit a signal, nor do ordinary content changes.
- Callbacks may unsubscribe during notification; newly registered callbacks start
  with the next change. Unsubscribe before destroying a view. Project destruction
  unsubscribes from directory notifications, and a weak lifetime token prevents
  already queued or late directory events from accessing the destroyed project.
- The existing known-file-type and CMake build-directory selection is shared by
  scans and new-file admission. The settings file is included once, even when an
  anonymous project's settings live outside the root. Configurable ignore rules
  and dynamic directory/watch reconciliation are still deferred.
- Changing the configured project root refreshes the model's file list, but does
  not reconfigure filesystem watches; watcher root changes belong to step 5.
- Twelve project tests cover refresh/add/remove, duplicate/equivalent paths,
  filtering, content-only changes, extension metadata, subscription changes,
  queued-event lifetime, and settings reload. Release build and both CTest entries
  passed in `build/json-upgrade`. No GUI styling or view behavior changes here.

## 4. Dedicated project tree

- [ ] Implement a specialized tree view rather than adapting the search-oriented
      `QuickList` or turning `ListView` into a tree.
- [ ] Build directory nodes from `Project::files()`, relative to the project root.
      This initially shows registered files and their parent directories, not
      every disk entry or empty directory.
- [ ] Support expand/collapse, selection, scrolling, clipped rendering, and mouse
      selection. Keep persistent selection/expansion keyed by path, not row index.
- [ ] Support arrows and h/j/k/l; Enter opens a file in the last active text editor.
- [ ] Subscribe to project-list changes and preserve navigation state on refresh.
- [ ] Add a toggleable "follow active file" mode: expand ancestors and scroll the
      active file into view without stealing keyboard focus. Handle files outside
      the project and files excluded from the list without errors.

Tests: hierarchy building, stable ordering, expansion, empty trees, navigation,
opening the correct path, state preservation, clipping, and follow mode on/off.
Test follow mode independently from focus changes.

## 5. Recursive filesystem notifications

- [ ] Implement recursive watching behind `DirectoryNotifications` in
      `src/linux/inotify.cpp`, not in the tree view.
- [ ] Track watch descriptors and their directory paths.
- [ ] Watch existing directories and directories created or moved into the tree.
- [ ] Handle directory deletion/moves and reconcile descendant project entries.
- [ ] Make watcher shutdown and changing the watched root safe.
- [ ] Handle queue overflow/watch failures with an explicit recovery or refresh
      strategy; avoid silently leaving the project index stale.

Tests: temporary-directory integration tests for nested add/remove/rename,
moving populated directories in/out, new directories, changing root, and clean
shutdown. Use bounded waits with useful failure messages rather than fixed sleeps.
Keep these tests separate from deterministic model/view tests.

## 6. Configurable project file selection

- [ ] Define include/exclude rules in `ProjectSettings` and document their syntax,
      defaults, path relativity, and precedence before implementing them.
- [ ] Centralize selection in the project layer, used for both scans and updates.
- [ ] Decide separately whether and how to honor `.gitignore`; recognizing the
      `.gitignore` filename currently does not mean its rules are applied.
- [ ] Preserve the current known-file-type/build-directory behavior until the new
      policy is explicitly chosen. Treat directory pruning separately from file
      filtering so supported descendants are not accidentally excluded.

Tests: included/excluded extensions and directories, nested paths, settings
reload, and identical results from initial scans and incremental updates.

## Validation and completion

For each implementation step:

1. Add focused tests in the existing `test/` setup and include new test sources in
   `test/tests.cpp` where appropriate.
2. Format changed C++ files with the project `.clang-format`.
3. Build the relevant targets and run `ctest --test-dir build --output-on-failure`.
4. Perform the GUI/CLI checks relevant to that step; report any checks that could
   not be performed. Mock-screen tests do not replace terminal-width testing.
5. Review the diff and Git status; keep the change separate from unrelated work.

Initial scope excludes drag-resizing, arbitrary docking/layout frameworks,
multiple sidebar tools, and persisted sidebar state. Add them only after the
basic sidebar and tree interactions are working.
