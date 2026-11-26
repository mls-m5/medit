# Follow-up: clangd output overwrites the CLI screen

Status: stderr is routed into the existing console as an error. Further work
is needed to investigate the reported compiler-invocation errors and to verify
real clangd behavior.

## Reported behavior

Opening a C++ file repeatedly produces clangd indexing errors directly over
ncurses' alternate screen, interleaved with the editor and project panel:

```text
Indexing .../build/.mm4/default/src/main.cpp.o failed: Couldn't build compiler invocation
Too many errors emitted, stopping now
```

The output must be captured by the application's logging/console routing, not
written directly to the terminal while ncurses owns the screen. Compiler errors
may remain useful diagnostics; suppressing them is not the routing fix.

## Original output-routing cause

In `lib/lsp-client/src/connection.cpp`, `Connection::startProcess()` redirects the
language server's stderr to `_errorPath`, a FIFO. However, `_errorThread` runs:

```cpp
std::system(("cat " + _errorPath.string()).c_str());
```

That child process inherits the application's OS-level stdout descriptor and
copies the FIFO straight to the terminal. In contrast, `src/core/meditlog.cpp`
captures C++ output by replacing `std::cout.rdbuf()` and `std::cerr.rdbuf()`.
These replacements do not capture a subprocess writing to inherited descriptors.
Changing the shell command to `cat ... >&2` alone would still bypass that capture.

## Later work

- The external `cat` has been replaced with a parent-process stderr reader and
  callback. The LSP plugin posts lines to the existing console as errors.
- Preserve the language server's stdout for LSP protocol messages. Do not redirect
  all process stdout/stderr indiscriminately: ncurses also needs terminal access.
- Exercise server shutdown and failure paths further. The connection still uses
  shell commands and named pipes; it does not forcibly terminate a stuck server.
- Investigate the compiler-invocation errors separately, starting with the chosen
  `compile_commands.json`, its source/output fields, and compiler flags. Paths
  ending in `.cpp.o` in the reported indexing messages are a clue, not a confirmed
  diagnosis of the compilation database.

## Acceptance checks

- Verify the complete host logging/console route using a fake language server.
  A connection-level fake server test already checks separate stderr delivery.
- Valid LSP stdout remains parseable, including while stderr is busy.
- Opening C++ files in `--cli` never lets child-process logs overwrite the screen.
- Connection shutdown does not hang, including missing/crashed server cases.
- GUI and GDB output behavior is checked too: `Connection` is shared infrastructure.

The real clangd failure was reported by the operator. It has not been reproduced
here because `clangd` is unavailable in the validation environment.
