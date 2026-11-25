# The best text editor in the world

![Alt text](docs/window.png?raw=true "Typical window with code")

![Alt text](docs/complete.png?raw=true "Example of autocomplete using clang")

## Features

- Lsp support for c++ (other languages not implemented yet)
- Vim keybindings


## Requirements

For a native Linux build: CMake 3.23 or newer, a C++20 compiler, Git (including
submodules), SDL2 with SDL2_ttf and SDL2_image development files, Boost.System,
and wide-character ncurses development files (`ncursesw`). CMake downloads
nlohmann/json 3.12.0 during configuration, so the first build needs network
access. For `medit --cli`, use a UTF-8 locale (check with `locale charmap`).

`clangd` is needed at runtime for C++ language-server features, but not to
build or run the editor. Git, `gdb` and `clang-format` are used by their
respective optional editor commands.

On Ubuntu 24.04, for example:

```sh
sudo apt update
sudo apt install build-essential cmake git libgl-dev libsdl2-dev \
    libsdl2-ttf-dev libsdl2-image-dev libboost-system-dev libncurses-dev
# Optional editor tools:
sudo apt install clangd gdb clang-format

git submodule update --init --recursive
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j2
ctest --test-dir build --output-on-failure
./build/medit --cli  # or ./build/medit for the GUI
```

## How to configure clangd

When building a project: add `-DCMAKE_EXPORT_COMPILE_COMMANDS=1` to your cmake
flags. Medit will find the compile_commands.json file that clangd uses to 
figure out compilation flags. You could also use some other type of generator
like bear, that uses any build system and create the json file.

If you want to have more build flags to your project, a tip is to look up how
clangd uses `.clangd` files in the project directory for local settings, or 
`~/.config/clangd/config.yaml` for global settings.

Example of information to put in config.yaml:
```
CompileFlags:
  Add: [-I/usr/include/c++/11, -I/usr/include/x86_64-linux-gnu/c++/11, -I/usr/include/c++/11/backward, -I/usr/lib/gcc/x86_64-linux-gnu/11/include, -I/usr/local/include, -I/usr/include/x86_64-linux-gnu, -I/usr/include]
```


Note that if you have multiplie compile_commands.json-files in your directory
Medit could pick the wrong one.



## Build with docker or podman

Install podman

```
sudo apt install podman
pip install podman-compose
```

build with 

```
./containerized_build.sh
```


[[todo]]
