# The best text editor in the world

![Alt text](docs/window.png?raw=true "Typical window with code")

![Alt text](docs/complete.png?raw=true "Example of autocomplete using clang")

Todo:
- [x] Show/hide console
- [x] Resize window
- [x] Quick-open
- [ ] Vim
  - [ ] Implement a lot of stuff
  - [x] Multi-letter vim combinations
  - [x] Visual mode
  - [x] Registers
  - [x] Full row visual mode
  - [ ] Implement all navigation shortcuts for all modes (iw aw 'i{' etc)
- [x] Undo/Redo
  - [x] Improve memory usage for undo/redo
- [x] Multiple open editors
- [x] Keep buffers open in background
- [x] Buffered screen output
- [x] Delayed syntax highlighting
- [x] Save files
- [ ] Find text in file
- [ ] Find text in project
- [ ] Ide-tools
  - [x] Switch headers/source
  - [ ] Find usages
  - [ ] Rename symbol
  - [x] Goto definition
  - [x] Switch header/source
  - [x] Auto complete
    - [x] Clang based autocomplete
  - [x] Basic syntax highlighting
    - [x] Clang-based syntax highlighting
- [x] Co-edit files with other people remotely
- [ ] Mouse control
  - [x] Mouse wheel control
  - [ ] Move cursor with mouse
  - [ ] Select with mouse drag
- [ ] File synch
  - [ ] Detect when open buffer is changed on disk
  - [ ] Prompt if buffer contains unsaved changes, otherwise replace
- User interface
  - [ ] Save unnamed buffers with popup with ctrl+s
  - [ ] Rename current document
  - [ ] File view
  - [ ] File name above buffers
  - [ ] Ctrl+tab switch between opened buffers
  - [ ] Layouts that handle splits
- [ ] ~~Use Lua~~
  - [x] Remove remainders of old scripting engine
  - [ ] ~~Make core functionality callable from lua~~
  - [ ] ~~Implement functions for keybindings in lua~~
  - [ ] ~~Replace mode logic with lua code~~
- [x] ctrl+backspace to erase whole word
- Editors
  - [ ] Word wrapping
  - [ ] Horizontal scroll


# Prioritized changes
- [ ] Faster navigation
- [x] Highlighting 
- [ ] Performance improvements for remote connections
   - [ ] Serialize to binary


## Feedback
- [ ] Ways to check if the lsp-plugin has the right plugin
- [ ] Make sure build command works properly, add settings
- [ ] Ways to handle settings
- [ ] Add searchable command pallette to make commands runnable (ctrl-p)
- [ ] CMakeLists.txt html and xml-file formatting
- [x] Select all (ctrl-a)
- [ ] Case insensitive matching in file list and command palette.

## Requirements

clangd for c++ parsing


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




