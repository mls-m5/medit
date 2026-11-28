# Project files and file system: redesign

Status: `ICommandRunner` (step 1) is on `main`. Steps 2–4 and 6 are
implemented on `feature/project-files` (P1–P4 in "Concrete plan" at the end);
step 5 (the changed-in-Git source) is not;
the operator approved that plan, including its performance-sensitive steps.
Buffer reloading on external changes was done separately
(`docs/file-reload-design.md`); it added `FileWatcher`, which this redesign
reuses instead of `IFileSystem::watch`.

## Goals

1. Buffers, `Files` and views do not talk to the operating system directly.
   All file access goes through one interface, `IFileSystem`.
2. The project file list comes from one selectable *file source*. The default
   is the files Git knows about; alternatives are all files in the directory
   (filtered) and the files changed in Git.
3. Consumers (project tree, locator, file browser, header switching, Markdown
   navigation) read one immutable snapshot and are told when it changes.

## Decisions made

- Default source at startup: `git ls-files` with tracked **and** untracked,
  non-ignored files (`--cached --others --exclude-standard`). Making the flags
  configurable is a later step.
- "Changed in Git" includes modified, staged, untracked and **deleted** files.
  Deleted files appear only in this mode, since it is the one used for diffs.
- The source is chosen through one menu (command palette) entry. The default is
  selected again when medit starts.
- Submodules are shown, with their files (see "Submodules" below).
- `.medit.json` is always part of the list, in every source.
- In the changed source, opening a file shows its diff in a read-only view
  that cannot be saved (like the console). Diff highlighting is designed later.
- The file limit stays, but becomes configurable.
- Symlinks are shown with a link marker in the tree (details below).
- `ICommandRunner` is separate from `IFileSystem`.
- Design first, then implement in small steps.

## Current state and its problems

| Area | Today | Problem |
|---|---|---|
| Buffers | `File` (`IFile`) reads/writes with `fstream` and `std::filesystem` | OS access inside the model; not testable without disk |
| `Files` | Owns the watcher; renames, checks existence directly | Same; reloads changed buffers even with unsaved edits (TODO) |
| Watcher | `DirectoryNotifications`, Linux inotify, root directory only | Started only when medit starts without a file argument; callbacks run on the *job* thread although the member is named `guiQueue`; a second `path()` call assigns over a running `std::thread` (terminates); non-Linux returns `nullptr`, which `Files` dereferences |
| `Project` | Recursive scan plus per-event edits of a `std::vector<path>` | Added paths skip the scan's filters; never unsubscribes; no change notification; extension counts not updated |
| Commands | `runCommand`/`runCommandAndCapture` (`core/os.h`, via `system()`), `POpenStream`, `Console`, `viewResultAsInteraction` (`plugin/git.cpp`) | Four ways to run a command, all taking shell strings with hand-written quoting (a `"` in a filename breaks `git add`); nothing a test can replace |

The incremental cache cannot stay consistent once the list comes from Git:
staging, `.gitignore` edits, checkouts and directory moves change membership
without a matching per-file event. The redesign therefore replaces "patch the
list per event" with "recompute the snapshot when something may have changed".

## Proposed structure

```text
 views (tree, locator, browser)      Files / Buffer / File
          │ snapshot + "changed"            │ read, write, stat, rename
          ▼                                 ▼
       Project ── FileSource ──────────► IFileSystem ◄── Linux / null / in-memory
                  (git, changed,            ▲ watch(root) → change hints
                   directory)               │
                      │                ICommandRunner (git) ◄── popen / fake
                      └────────────────────►┘
```

### `IFileSystem`

One interface for everything that touches files. Sketch:

```cpp
struct FileInfo {
    bool exists = false;
    bool isDirectory = false;
    std::filesystem::file_time_type modified;
    std::uintmax_t size = 0;
};

class IFileSystem {
public:
    virtual ~IFileSystem() = default;

    virtual std::optional<std::string> read(const Path &) = 0;
    virtual bool write(const Path &, std::string_view content) = 0;
    virtual bool rename(const Path &from, const Path &to) = 0;
    virtual FileInfo stat(const Path &) = 0;

    // Recursive listing used by the directory source. Never follows
    // directory symlinks; skips unreadable directories.
    virtual std::vector<Path> list(const Path &root,
                                   const ListFilter &filter) = 0;

    // Calls `hint` on `queue` when something below `root` may have changed.
    // Hints are coarse; receivers recompute rather than trust them.
    virtual WatchHandle watch(const Path &root,
                              IJobQueue &queue,
                              std::function<void(const Path &)> hint) = 0;
};
```

- `WatchHandle` unsubscribes when destroyed. This removes the `void *ref`
  bookkeeping and the lifetime problems around `this` captures.
- The caller chooses the queue for callbacks, which fixes today's
  wrong-thread delivery.
- Implementations: Linux (std::filesystem plus inotify), a null implementation
  for platforms without watching (watch is a no-op, never `nullptr`), and an
  in-memory implementation for tests.
- `File` gets its content through `IFileSystem`. Buffers keep using `IFile` and
  never see paths being read or written. The one exception today,
  `Buffer::open()` constructing a `File` and calling
  `std::filesystem::is_regular_file`, moves to `Files`. If a buffer later
  needs to know whether its file changed on disk, `IFile` answers and its
  implementation asks `IFileSystem`.
- Process execution is not a file operation and gets a separate
  `ICommandRunner`; see below.

### `ICommandRunner`

```cpp
struct CommandResult {
    int exitCode = -1;
    std::string output;   // stdout
    std::string errors;   // stderr, kept apart from the protocol output
};

struct Command {
    std::vector<std::string> arguments; // arguments[0] is the program
    Path workingDirectory;

    // For commands the user writes in .medit.json (build, run, format):
    // these are meant to be shell syntax.
    static Command shell(std::string commandLine, Path workingDirectory) {
        return {{"/bin/sh", "-c", std::move(commandLine)},
                std::move(workingDirectory)};
    }
};

// Optional line callbacks. A stream with a callback is delivered line by
// line (without the newline) and is not collected in CommandResult.
struct CommandOutput {
    std::function<void(std::string_view)> output;
    std::function<void(std::string_view)> errors;
};

class ICommandRunner {
public:
    virtual ~ICommandRunner() = default;
    // Blocking; prefer the job thread. No shell unless the command is built
    // with Command::shell, so paths are passed as-is and need no quoting.
    // Callbacks run on the calling thread.
    virtual CommandResult run(const Command &,
                              const CommandOutput &stream = {}) = 0;
};
```

Callers today fall into three groups:

| Group | Where | Change |
|---|---|---|
| Paths pasted into shell strings | `git.cpp` (add, blame, push), `gitcommitinteraction.cpp` (rm, add, commit), `genericformat.cpp` (six formatters) | Argument lists; this is where quoting breaks today |
| User-written commands | build, run and format commands from `.medit.json` | `Command::shell`, with an explicit working directory |
| Streaming output | `Console` (build, run, git push) | Later: a line-callback variant of `run` |

`fifofile.cpp` runs `mkfifo` through `system()`; it should call `::mkfifo()`
instead and needs no runner at all.

- Separate from `IFileSystem` because the concerns differ: file-system tests
  do not need a command fake, and commands are also used by formatting, build
  and Git plugins that never read project files.
- The cost is that a remote setup must provide a matching pair. Both are
  therefore obtained from the same place (the core environment), never created
  ad hoc, so they can be swapped together later.
- Existing helpers move to it gradually; this redesign only requires the file
  sources to use it. Streaming output to the console (`Console`) can stay as it
  is until it is migrated separately.

### File sources

```cpp
struct ProjectFile {
    Path path;               // absolute, lexically normal
    GitStatus status = {};   // only filled by the changed source
};

struct ProjectFiles {
    Path root;
    std::vector<ProjectFile> files;   // sorted, unique
};

class FileSource {
public:
    virtual ~FileSource() = default;
    virtual std::string_view name() const = 0;
    // Runs on the job thread; must not touch editor state.
    virtual ProjectFiles list(const Path &root) = 0;
};
```

| Source | Command / method | Notes |
|---|---|---|
| Git files (default) | `git -C root ls-files -z --cached --others --exclude-standard` | Tracked files missing from disk are dropped (`--deleted` or `stat`). Flags configurable later |
| Changed in Git | `git -C root status --porcelain=v1 -z --untracked-files=all` | Modified, staged, untracked, deleted, unmerged; a rename lists the new path. Keeps the status for later coloring |
| Directory | `IFileSystem::list` with a filter | Today's rules: known extensions, skip CMake build directories, skip `.git`. Filter configurable later |

- Use `-z` output, so filenames with spaces, newlines or invalid UTF-8 survive.
- `.medit.json` is always added to the snapshot, whatever the source returns.
- The number of files is capped by a configurable limit (default: today's
  100 000). The cap exists so a huge directory cannot hang the editor; when it
  is reached, the status bar says the list is incomplete.

### Submodules

`git ls-files --recurse-submodules` only supports `--cached`, not `--others`
(checked: "unsupported mode"). The Git sources therefore recurse themselves:

1. List the repository as usual. Submodules appear as single entries; they are
   identified with `git ls-files -z --stage` (mode `160000`).
2. For each initialized submodule, run the same source with the submodule as
   root and prefix the paths. Repeat for nested submodules.
3. An uninitialized submodule is shown as an empty directory with a submodule
   marker.

For the changed source, `git status` reports a submodule with new commits or
local changes as one modified entry; the files inside come from running
`git status` in the submodule. The directory source treats submodules as
ordinary directories. The tree may mark submodule roots.

### Diff view for changed files

Opening a file from the changed source opens a read-only buffer (an
`UnsavableFile`, as the console uses) holding its diff:

| Status | Command |
|---|---|
| Modified, staged or both | `git diff HEAD -- path` |
| Untracked | `git diff --no-index -- /dev/null path` |
| Deleted | `git diff HEAD -- path` |
| Submodule | `git diff --submodule=log HEAD -- path` |

`git diff --no-index` exits with 1 when the files differ, which is the normal
case here; only other exit codes are errors. Opening the real file stays
possible from the other sources. For highlighting,
`universalGitFormat` in `plugin/git.cpp` already colors Git output and can serve
as a starting point; proper diff highlighting is a separate design.

### Symlinks

- Git lists a symlink as one entry, also when it points to a directory.
- A symlink to a file is shown with a single-column link marker and opens its
  target.
- A symlink to a directory is shown with the marker but is not expanded in the
  first version, to avoid loops and paths outside the root.
- The directory source never follows directory symlinks.
- If the root is not in a Git repository, or `git` is missing, use the directory
  source and say so in the status bar.

### `Project`

- Owns the selected source and the latest `ProjectFiles` snapshot.
- `refresh()` runs the source on the job queue and swaps the snapshot on the
  GUI thread. Refreshes are coalesced: one running and at most one pending.
- `subscribeToFilesChanged(callback)` returns a handle; callbacks run on the
  GUI thread only when the snapshot differs from the previous one.
- The extension statistics used to guess the language are derived from each
  new snapshot.
- Settings handling (`ProjectSettings`) stays as it is in this redesign.

### When to refresh

- When the project opens, when the source is changed, and through an explicit
  `refresh_project_files` command.
- After medit writes or renames a file (`Files` knows).
- From watch hints, debounced (around 200 ms), on the working tree and on
  `.git/index` and `.git/HEAD`. Staging and commits change membership without
  touching the working tree.
- Never during drawing.

## Migration steps

Each step is one branch, built and tested on its own.

1. **`ICommandRunner`** with a Linux implementation (`posix_spawn`, separate
   stdout/stderr pipes read without deadlock) and a fake. Migrate the Git
   commands that take paths, with a regression test for a filename containing
   quotes. Formatters, `mkfifo` and the console follow as separate small
   steps.
2. **`IFileSystem`** with Linux, null and in-memory implementations. `File` and `Files` use them. The old
   `DirectoryNotifications` stays behind an adapter for now. No intended
   behavior change, except that non-Linux builds get a null watcher instead of
   `nullptr`. *Touches watch dispatch: approval needed.*
3. **Snapshot, `FileSource` and subscription.** The directory source reproduces
   today's scan. `Project` exposes `ProjectFiles`. Callers are adapted: tree,
   locator, browse interaction, Markdown navigation, header switching. Per-event
   `add`/`removeCachedFile` is removed. The tree subscribes instead of refreshing
   on F3. *Touches scanning: approval needed.*
4. **Git source as default**, with the fallback to directory.
5. **Changed-in-Git source**, the menu entry for choosing a source, and the
   read-only diff view.
6. **Watch-driven refresh**: debounced hints, recursive watching, `.git/index`.
   Remove `DirectoryNotifications`. *Approval needed.*

Later, separately: buffer reload/conflict handling on external changes through
`IFileSystem::stat`; Git status colors in the tree; configurable flags and
filters in `.medit.json`.

## Tests

- In-memory `IFileSystem` and fake `ICommandRunner`: parsing of `ls-files` and
  `status` output (including `-z`, renames, deletions, odd names), fallback,
  sorting/uniqueness, "changed" only on real membership change.
- `Project` refresh coalescing and subscription lifetime, with deterministic
  job/GUI queues (no sleeps).
- Linux implementation: temporary-directory integration tests with bounded
  waits, kept separate from the deterministic tests.
- CLI smoke tests for the tree after each user-visible step.

## Open questions

1. **Link marker**: which single-column glyph? It must not be ambiguous-width,
   or it misaligns in East Asian terminals.
2. **Diff view details**: one view per file, or one that is replaced when
   another changed file is selected? Staged and unstaged together (as above)
   or separately?
3. **Submodule cost**: listing runs one or two `git` calls per submodule. Fine
   for a handful; should there be a setting to skip them in large trees?

## Concrete plan (`feature/project-files`)

Built on `fix/cli-autodetect`, which has the reload work (`FileWatcher`,
`DiskContent`, `File` baselines). One commit per step, each built and tested.

### P1. `IFileSystem` without watching

```cpp
struct FileInfo { bool exists; bool isDirectory; bool isSymlink; };
class IFileSystem {
    virtual std::optional<std::string> read(const Path &) = 0;
    virtual bool write(const Path &, std::string_view) = 0;
    virtual FileInfo stat(const Path &) = 0;
    virtual std::vector<Path> list(const Path &root, const ListFilter &) = 0;
};
```

- `LocalFileSystem` (std::filesystem) and `MemoryFileSystem` for tests.
- `File` reads and writes through it (`readDiskContent` becomes a call on it);
  `CoreEnvironment::fileSystem()` owns the local one.
- Watching stays in `FileWatcher` (directories, own thread), already used for
  buffers; a second watch interface would duplicate it.

### P2. Snapshot, `FileSource` and subscription

- `ProjectFiles { Path root; std::vector<Path> files; bool truncated; }`,
  sorted and unique, `.medit.json` always included.
- `FileSource::list(root, limit)` runs on the job thread. `DirectorySource`
  reproduces today's scan through `IFileSystem::list` (known extensions, skip
  CMake build directories and `.git`).
- `Project` owns the source and the snapshot. `refresh()` lists on the job
  queue and swaps on the GUI thread; one running, at most one pending.
  `subscribeToFiles()` returns a handle; callbacks run on the GUI thread when the
  list actually changed. The first list is made synchronously at startup, as
  today. Extension statistics come from each snapshot.
- The per-event `add/remove/updateCachedFile` go away. `.medit.json` is
  reloaded when the file watcher reports it.
- Callers: the tree subscribes; the locator, browse interaction, Markdown
  navigation and header switching read the snapshot. Opening the tree or the
  locator asks for a refresh instead of rescanning on the GUI thread.

### P3. Git source as default

- `git ls-files -z --cached --others --exclude-standard` through
  `ICommandRunner`, in the project root. Tracked files missing on disk are
  dropped.
- Submodules: an entry that is a directory with a `.git` is listed with the
  same source and prefixed; nested ones too.
- Not a repository, or no `git`: the directory source, with a status message.
- Reaching the limit marks the snapshot truncated and says so.

### P4. Refresh from watch hints

- `Project` watches the root, the directories in the snapshot (capped, e.g.
  2000, to stay within inotify limits) and `.git` (index and HEAD changes from
  staging, commits and checkouts) with a `FileWatcher`.
- Hints are debounced (about 200 ms) into one refresh. In `.git`, only
  `index` and `HEAD` count.
- `DirectoryNotifications` is removed (done already in P2, once nothing used
  it).

### Step 5 (`feature/changed-files-source`)

C1. **Changed source.** `git status --porcelain=v1 -z --untracked-files=all`
    through the command runner, parsed with `parseGitStatus`. Modified,
    staged, untracked, unmerged and deleted files; a rename lists the new
    path. Initialized submodules are listed with their own changed files.
    `ProjectFiles` gets the two-letter status of each file (`status`), empty
    for the other sources. Outside a repository the list is empty and the
    status bar says so.

C2. **Choosing the source.** `Project::source(FileSourceKind)` with `Git`
    (default at every start), `Changed` and `Directory`; changing it refreshes
    the list. Three palette commands: `project_files_git`,
    `project_files_changed`, `project_files_directory`. The tree's heading
    names the source (" Project", " Changed in Git", " Directory").

C3. **Diff view on open.** With the changed source, opening a file from the
    tree or the locator shows its diff (the existing `git_diff_file` view,
    read-only) instead of the file, which also works for deleted files. The
    diff view code moves from `plugin/` to `script/`, since the views cannot
    depend on plugins and the code only needs the core.

Not in this step: status colors in the tree, the symlink marker, diff
highlighting beyond the current line colors.

### Later

The symlink marker, configurable flags, filters and limit.

