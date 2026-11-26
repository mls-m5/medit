# Project tree: requirements and proposed first increment

**Draft for review, not approval to implement.** This design starts from
`main`. The existing implementation on `terra-destroying-the-codebase` is a
reference, not a set of commits to merge wholesale.

## Requirements already established

1. The panel is optional and initially hidden. It must not reduce editor space
   while hidden, overlap editor splits, the console or the status line, or leave
   keyboard focus in an invisible panel after a resize.
2. The tree displays project files and their parent directories. `Project` owns
   file membership; the view must not recursively scan the filesystem, call
   Git, or query file metadata for each row or redraw. Empty directories and
   files outside the project root do not become tree entries.
3. A selected file opens through the existing editor/file-opening path, in the
   active text-editor split. Tool keys and paste must not edit a background
   text buffer. Unchanged files keep their ordinary styling.
4. Both CLI and GUI must handle selection, clipping and resize. Terminal labels
   must not misalign the panel when filenames contain double-width, combining,
   control, or invalid UTF-8 characters; the path used for opening is never
   altered to match its display label.
5. Changes to project scanning, notification dispatch, screen drawing or other
   performance-sensitive paths need explicit approval before implementation.
   Build and test each increment separately; keep unrelated submodules intact.

## Proposed smallest useful increment (requires your approval)

- Add a fixed-width, toggleable sidebar, hidden at startup. F3 shows/focuses or
  hides it; Escape returns focus to the editor. On narrow windows the panel
  temporarily yields its space to the editor. Reuse existing window/view/screen
  abstractions; no new screen backend.
- At panel opening, build an in-memory tree from one snapshot of
  `Project::files()`. Create only implicit parent directories of those files,
  ordered directories-first and then by filename. Keep selection and expansion
  keyed by path, not row number. Do not change the model's file-selection rules.
- Support Up/Down (or j/k), Left/Right (or h/l), Enter to expand/collapse or
  open a file, and scrolling. Display a clear selected row. No filesystem or
  Git work during drawing. Refresh the snapshot only when opening the panel,
  **not** on every redraw; decide whether opening should invoke the existing
  full project refresh or simply use its current cache before coding it.
- Add focused tests for hierarchy, file opening, focus restoration, empty and
  narrow layouts, resize, and UTF-8-safe clipping. Check both GUI and CLI where
  feasible. Documentation of controls belongs in `docs/usage.md` when built.

## Separate increments, not part of the first review

- Live membership updates: project-owned notifications and recursive watches,
  with failure recovery and tests. The current project cache does not provide a
  reliable recursively watched tree. Do not copy the watcher rewrite as part of
  the initial UI change.
- Mouse opening, F6 focus toggle, follow-active-file, configurable width,
  persisted state, and cosmetic styling beyond the selected row.
- Git change coloring: a cached status snapshot refreshed outside drawing;
  previously requested scope was changed files including staged and untracked,
  but not ignored files. This must not introduce files into the tree.

## Decisions needed before implementation

1. **Git-ignored files:** you expressed a preference that they not appear at
   all. Today's `main` project file list does not apply `.gitignore` rules.
   Should hiding them be a prerequisite for the first tree, or a separate
   project-selection feature? Simply omitting Git *color* for ignored entries
   would not satisfy this preference.
2. **Refresh at F3:** is one potentially costly project scan when opening the
   panel acceptable, or should the first version show only the current cache
   until an explicit refresh? Never scan on redraw.
3. **First-version controls:** are keyboard-only navigation and F3/Escape
   sufficient for the first review, with mouse and F6 in a later increment?
4. **Layout:** is a fixed initial width (for example 24 content columns and a
   divider, subject to a minimum editor width) acceptable for the prototype?

No changes to `Project`, the watcher, Git integration or editor behavior are
proposed until these choices are reviewed.
