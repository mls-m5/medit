# Design: reloading buffers when files change on disk

Status: proposal for review. Nothing here is implemented yet. The steps that
touch file watching, `Files` or notification dispatch are performance-sensitive
and need explicit approval before they are implemented.

Related: `docs/project-design.md` (on branch `docs/project-design`) describes
`IFileSystem`, `WatchHandle` and the project file sources. This design uses the
same ideas but does not depend on that work landing first.

## Goals

1. When another program changes a file that is open in medit (Git checkout,
   formatter, another editor, code generator), the buffer follows the file.
2. Unsaved changes in memory are never lost. A buffer with unsaved changes is
   only updated when the external change can be merged without touching what
   the user changed; otherwise the buffer is left alone and marked.
3. medit's own saves never trigger a reload.
4. Every reload or merge is one undo step, so it can be taken back.
5. The GUI thread never waits for disk reads or diffs.

Non-goals for now: reloading files that are not open, recursive project
watching (step 6 of `project-design.md`), binary files, interactive merge
tools.

## Today

| Part | Behavior | Problem |
|---|---|---|
| `Files::fileChangeCallback` | On `Changed`/`Added` for an open file, calls `Buffer::load()` | Overwrites unsaved changes (the code has a TODO for this) |
| Own saves | `File::save` writes, then inotify reports `IN_MODIFY` | The buffer reloads its own content. Text typed between the save and the event is lost |
| Watcher | `DirectoryNotifications`, project root only, not recursive | Files in subdirectories are never reloaded. Callbacks are delivered on the job queue although the member is called `guiQueue`; a second `path()` assigns over a running `std::thread` |
| Deleted or renamed files | Ignored | The buffer silently refers to a missing file |
| `Buffer::load` | Replaces the whole text through `replace(all(...))` | Already one history entry, but the cursor stays at the same coordinates even when lines above it changed |

## Concepts

### Disk baseline

Every buffer that has a file remembers what it last agreed with on disk: the
result of the last load or save.

```cpp
struct DiskBaseline {
    bool exists = false;
    std::filesystem::file_time_type modified;
    std::uintmax_t size = 0;
    std::size_t hash = 0;  // Of the bytes on disk
    std::string text;      // The buffer text those bytes load as (merge base)
    long revision = 0;     // History revision when the baseline was taken
};
```

- Set after `load` (from the bytes read) and after `save` (from the bytes
  written). This is what makes our own saves invisible: the change event that
  follows a save finds the disk equal to the baseline and is dropped.
- `text` is the common ancestor for a three-way merge. It doubles the memory of
  an open file; files above a size limit (see below) keep only the hash and
  are never merged.
- "Loads as" matters because `File::save` appends a final newline: the text is
  produced by the same function `File::load` uses, so comparisons never see a
  spurious difference in the last line.
- It lives next to the `IFile` (owned by the buffer), so it follows renames.

### Buffer disk state

```cpp
enum class DiskState {
    InSync,          // Disk equals the baseline
    ChangedOnDisk,   // Disk differs and could not be merged; buffer untouched
    DeletedOnDisk,   // The file is gone; buffer content kept
};
```

Shown in the status line and window title (e.g. `name - medit [changed on
disk]`). `isChanged()` keeps its meaning (unsaved edits in memory).

## Detecting changes

Three sources, all of which only produce a *hint* ("this path may have
changed"). The receiver always re-checks the disk, so missed, duplicated or
reordered events do not matter.

1. **Watching the directories of open buffers.** One watch per directory that
   contains an open file, reference counted, added when a buffer opens and
   removed when it closes. Directories rather than files, because many programs
   save by writing a temporary file and renaming it (vim, formatters, `git
   checkout`), which replaces the inode a file watch would hold. Events needed:
   `IN_CLOSE_WRITE`, `IN_MOVED_TO`, `IN_CREATE`, `IN_DELETE`, `IN_MOVED_FROM`.
   `IN_CLOSE_WRITE` instead of `IN_MODIFY` avoids reacting to every partial
   write.
2. **Before saving.** `save` checks the disk against the baseline first, so a
   change the watcher missed (network file systems, non-Linux builds) cannot be
   overwritten silently.
3. **When a buffer is shown again** (switching to it or reopening it through
   `Files::open`) a cheap `stat` check. This is the fallback on platforms
   without a watcher.

Hints for the same path are debounced (about 100 ms) and coalesced, so a
formatter writing in several chunks or a checkout of many files causes one
check per file.

## Handling a change

```
watch / save / show ──hint(path)──► job thread                     GUI thread
                                    stat(path)
                                    equal to baseline? ── drop
                                    read bytes, hash
                                    equal hash? ─────────────────► refresh baseline
                                    decide (below) ──────────────► apply if the buffer's
                                                                   revision and baseline
                                                                   are still the ones used
```

The job thread works on a copy of the buffer text and the baseline taken on the
GUI thread. When the result arrives on the GUI thread it is applied only if the
buffer's history revision and baseline are unchanged; otherwise the check is
rescheduled. This handles typing or saving while a check is in flight.

### Decision

| Buffer | Disk | Action |
|---|---|---|
| No unsaved changes | Changed | **Reload.** Replace only the changed line range (common prefix and suffix kept), as one history step. New baseline. Message: `reloaded README.md` |
| Unsaved changes | Changed, edits do not overlap | **Merge.** Apply the disk side's hunks to the buffer as one history step. New baseline = disk; the buffer stays modified. Message: `README.md changed on disk; merged with unsaved changes (undo to revert)` |
| Unsaved changes | Changed, edits overlap | **Leave the buffer alone.** State `ChangedOnDisk`. Message: `README.md changed on disk and conflicts with unsaved changes` |
| Any | Deleted or moved away | State `DeletedOnDisk`, content kept, marked as unsaved so the content is not dropped by accident. Saving recreates the file |
| Any | Deleted, then recreated (atomic save) | Treated as changed: the check runs after the debounce and sees the new file |
| Too large or not valid UTF-8 | Changed | Unmodified buffer: reload as a whole. Modified: `ChangedOnDisk`, no merge |

Should a merge be automatic (row 2) or always ask? This proposal merges
automatically because it is one undo step and the message says so. It is a
setting if you prefer to be asked.

### The merge

A line-based three-way merge, like `git merge-file`:

- `base` = baseline text, `ours` = buffer text, `theirs` = disk text.
- Diff `base→ours` and `base→theirs` with Myers' algorithm on lines, after
  trimming the common prefix and suffix (which makes typical cases nearly
  linear).
- Hunks from `theirs` that do not overlap or touch a hunk from `ours` are
  applied; identical changes on both sides count as no conflict. Anything else
  is a conflict and nothing is applied.
- The result is a list of line edits on `ours`, applied as `BufferEdit`s within
  one major history step.

This is a pure function on strings, so it is tested without files or threads:

```cpp
struct LineHunk { size_t begin; size_t end; std::vector<std::string> lines; };
struct MergeResult { bool conflict = false; std::vector<LineHunk> editsToOurs; };
MergeResult mergeLines(std::string_view base,
                       std::string_view ours,
                       std::string_view theirs);
```

The same line diff can produce a unified diff for the "compare with disk"
command below, displayed with the existing `formatGitDiff` view.

### Cursors

Reload and merge apply only the changed ranges, so text and cursors outside
them stay as they are. A cursor below a changed range is moved by that hunk's
line delta, so it stays on the same text instead of the same line number. A
cursor inside a replaced range stays at the same line, clamped. This needs a
hook from the applied edits to the editors that show the buffer; today nothing
adjusts other editors' cursors when a buffer changes, so this is new.

## Commands

| Command | Effect |
|---|---|
| `reload_file` | Discard unsaved changes and load from disk (one undo step) |
| `diff_with_disk` | Read-only view: unified diff between disk and buffer |
| `save` | Refuses when the state is `ChangedOnDisk` and says why |
| `save_overwrite` | Saves anyway, replacing the disk version |

## Where the code goes

- `src/text/linemerge.{h,cpp}`: line diff, `mergeLines`, unified diff text. Pure.
- `IFile` / `File`: read and write return the bytes, the baseline is computed
  there (`File` is the only place that knows how bytes become buffer text).
  When `IFileSystem` lands, `File` asks it for `stat`/`read`.
- `Files`: owns the per-directory watches, the debounce and the decision. It
  already owns the buffer list and the watcher. The project-root watcher used
  by `Project` stays as it is until `project-design.md` step 6.
- `Buffer`: holds the `DiskState`, and the hook that reports applied line
  deltas to its editors.
- MainWindow: no new logic. It only shows the state in the title it already
  builds.

## Steps

Each step is one branch with its own tests.

1. **Line merge**: `linemerge` with unit tests. No behavior change.
2. **Baseline and safe save**: baseline after load/save; `save` refuses when the
   disk changed; `save_overwrite`; `DiskState` in the title. Fixes losing an
   external change by saving over it. *Touches `File`/`Buffer` save and load.*
3. **Reload pipeline**: per-directory watches with `IN_CLOSE_WRITE`, debounce,
   job-thread check, GUI-thread apply with the revision check. Unmodified
   buffers reload minimally; modified buffers become `ChangedOnDisk` (no merge
   yet). Fixes today's overwrite of unsaved changes and the own-save race.
   *Touches watch dispatch: approval needed.*
4. **Merge**: use `mergeLines` for modified buffers. `reload_file`,
   `diff_with_disk`.
5. **Cursor adjustment** across reload and merge.
6. Deleted and moved files, and the `stat` check when a buffer is shown again.

Steps 2 and 3 already satisfy "never reload a buffer with unsaved changes".
Steps 4–6 add the merging.

## Tests

- `mergeLines`: non-overlapping edits, overlapping edits, identical edits on
  both sides, insertions at the same line, edits at the start and end, empty
  files, missing final newline, CRLF lines kept as they are.
- `Files` with a temporary directory and deterministic job/GUI queues (no
  sleeps): unmodified reload; merge; conflict leaves the buffer untouched; own
  save ignored; typing between save and the change event is kept (regression
  for today's race); the result is dropped and rechecked if the buffer changed
  in flight; deleted file; undo restores the text from before a reload or
  merge.
- Watcher integration test in a temporary directory with bounded waits, kept
  separate: atomic rename save, `IN_CLOSE_WRITE`, file in a subdirectory.
- tmux: open a file, change it from outside with and without unsaved edits,
  check the text, the message and undo.

## Open questions

1. Merge automatically when there is no conflict, or always ask first?
2. On conflict: only mark and refuse to save (proposed), or also offer a
   command that inserts Git-style conflict markers into the buffer?
3. Size limit for keeping the merge base in memory (proposal: 4 MB per file).
4. A deleted file: keep the buffer marked unsaved (proposed), or close it if it
   had no unsaved changes?
