# Project files and file system: redesign

**Draft for review, not approval to implement.** Each step below is a separate
branch and needs its own review. Steps touching scanning, watching or
notification dispatch are performance-sensitive and need explicit approval.

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
- Design first, then implement in small steps.

## Current state and its problems

| Area | Today | Problem |
|---|---|---|
| Buffers | `File` (`IFile`) reads/writes with `fstream` and `std::filesystem` | OS access inside the model; not testable without disk |
| `Files` | Owns the watcher; renames, checks existence directly | Same; reloads changed buffers even with unsaved edits (TODO) |
| Watcher | `DirectoryNotifications`, Linux inotify, root directory only | Started only when medit starts without a file argument; callbacks run on the *job* thread although the member is named `guiQueue`; a second `path()` call assigns over a running `std::thread` (terminates); non-Linux returns `nullptr`, which `Files` dereferences |
| `Project` | Recursive scan plus per-event edits of a `std::vector<path>` | Added paths skip the scan's filters; never unsubscribes; no change notification; extension counts not updated |
| Git | Plugins run `git` through `POpenStream` | No shared way to run commands that a test can replace |

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
  never see paths being read or written.
- Process execution is not a file operation and gets a separate
  `ICommandRunner` (run a command in a directory, return output and exit code)
  that wraps `POpenStream`. A future remote file system can provide both.

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

1. **`IFileSystem` and `ICommandRunner`** with Linux, null and in-memory
   implementations. `File` and `Files` use them. The old
   `DirectoryNotifications` stays behind an adapter for now. No intended
   behavior change, except that non-Linux builds get a null watcher instead of
   `nullptr`. *Touches watch dispatch: approval needed.*
2. **Snapshot, `FileSource` and subscription.** The directory source reproduces
   today's scan. `Project` exposes `ProjectFiles`. Callers are adapted: tree,
   locator, browse interaction, Markdown navigation, header switching. Per-event
   `add`/`removeCachedFile` is removed. The tree subscribes instead of refreshing
   on F3. *Touches scanning: approval needed.*
3. **Git source as default**, with the fallback to directory.
4. **Changed-in-Git source** and the menu entry for choosing a source.
5. **Watch-driven refresh**: debounced hints, recursive watching, `.git/index`.
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

1. **Submodules**: `ls-files` lists a submodule as one entry. Show it as a
   directory without contents, skip it, or list its files (another command per
   submodule)?
2. **`.medit.json`**: today it is always appended. Keep that in every source?
   In the changed source too?
3. **Opening a deleted file** in the changed source: show nothing for now, or
   open an empty read-only buffer until a diff view exists?
4. **Size limit**: today's scan stops at 100 000 files. Keep a limit for all
   sources?
5. **Symlinks**: Git lists a symlink as a file. Open the target, or show it
   but not open directories?
6. **Where `ICommandRunner` lives**: next to `IFileSystem` (so a remote file
   system also runs Git remotely) or as an independent service?
