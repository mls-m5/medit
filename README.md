The best text editor in the world
======================================

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
  - [ ] Full row visual mode
  - [ ] Implement all navigation shortcuts for all modes (iw aw 'i{' etc)
- [x] Undo/Redo
  - [ ] Improve memory usage for undo/redo
- [x] Multiple open editors
- [ ] Keep buffers open in background
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
- [ ] Co-edit files with other people remotely


Requirements
-------------------------------
  
For legacy clang plugin: (Its deprecated)

build with -DENABLE_LEGACY_CLANG_PLUGIN
libclang-10/9
