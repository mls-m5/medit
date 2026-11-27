# Building and dependency versions

Native Linux builds need CMake 3.23 or newer, a C++20 compiler, Git submodules,
SDL2/SDL2_ttf/SDL2_image development packages, Boost.System, and wide
ncurses (`ncursesw`) development headers. `--cli` also needs a UTF-8 locale
(e.g. `C.UTF-8`) for correct terminal input and cursor placement. Initialize
submodules with `git submodule update --init --recursive` before configuring. A fresh CMake configure downloads
nlohmann/json 3.12.0; it needs network access.

This build-compatibility branch pins `lib/json.h` to `009d642` (its CMake
minimum-version update). `lib/lsp-client` is no longer a submodule: its full history is merged into
this repository (subtree merge of upstream through `e66e0bb`).
The GitHub Actions native build installs `libncurses-dev`; the container build
uses Ubuntu 24.04 and installs the same package. Test dependency upgrades with
a fresh build directory as well as the existing test suite.
