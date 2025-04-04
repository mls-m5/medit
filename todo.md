# Todo
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
  - [ ] r-command (replace single character)
  - [ ] Generalized way to handle c, d, y and motion commands
  - [ ] ctrl+O ctrl+I to navigate back and fourth
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
  - [x] Rename symbol
  - [x] Goto definition
  - [x] Auto complete
    - [x] Clang based autocomplete
  - [x] Basic syntax highlighting
    - [x] Clang-based syntax highlighting
- [x] Co-edit files with other people remotely
- [ ] Mouse control
  - [x] Mouse wheel control
  - [x] Move cursor with mouse
  - [ ] Select with mouse drag
- [ ] File synch
  - [x] Detect when open buffer is changed on disk
     - [ ] Prompt if buffer contains unsaved changes, otherwise replace
- User interface
  - [x] Save unnamed buffers with popup with ctrl+s
  - [x] Jump to file under cursor (path, markdown links etc) `[[braces syntax]]`
  - [x] Rename current document
  - [x] File view
  - [ ] File name above buffers
  - [ ] Ctrl+tab switch between opened buffers
  - [ ] Layouts that handle splits
  - [ ] Use formatting (palette) to handle grouping ({}) or hyperlink marking
  - [ ] Prompt if the user want to save beforeo exits
  - [ ] Prompt to diff pasted data to data in editor (more than per line)
      - [ ] Same logic can be used to view git diffs in editor
  - [ ] Toggle markdown crosses with space
- [ ] ~~Use Lua~~
  - [x] Remove remainders of old scripting engine
  - [ ] ~~Make core functionality callable from lua~~
  - [ ] ~~Implement functions for keybindings in lua~~
  - [ ] ~~Replace mode logic with lua code~~
- [x] ctrl+backspace to erase whole word
- Editors
  - [x] Word wrapping
  - [x] Horizontal scroll
- [ ] Performance improvements for remote connections
   - [ ] Serialize to binary
- [ ] Communication over network sessions
   - [ ] Add interactions for users to add own name
   - [ ] Create some way for users to send short messages to eachother.
   - [ ] Show user cursors on other users screen
- [ ] Git plugin
   - [x] Push
   - [ ] Diff
   - [x] Status
   - [x] Blame
   - [x] Add
   - [x] Commit (interaction where you select which files and write message)
- [x] GDB debugger integration
   - [x] Basic operations, run, set breakpoints step
   - [x] Visualize breakpoints (using annotation system)
- [ ] Console
   - [x] Interact with console (focus, scroll, keypresses etc)
   - [ ] Support for multiple consoles for different usages.
       - [ ] Implement it
       - [ ] Ways to switch between consoles
       - [ ] Open interactions in console as standard (but add option to open in current editor)
   - [ ] Support backspace characters for popen
   - [ ] Make Popenstream support colors in linux
- [ ] Lsp-plugin
    - [ ] Lsp-plugin diagnostics
       - [ ] Command that prints state of lsp-server and also command flags for
             current document
    - [ ] Fix: Lsp urli-s with special characters is not handled properly
    - [ ] Add refactor function (alt+return)
- [ ] Distraction free mode, where you are only allowed to edit the end or the last lines
- [ ] Word wrap (soft)
- [ ] Standard commands that is only available to certain file types
- [ ] Handle problems with separate sources, 
      make it so that each source replace its content when there is something new
      for example lsp-plugin, the compiler or formatter (if it emits error messages)
- [ ] Show error if formatting does not work


## Prioritized changes

- [ ] Faster navigation
   - [ ] Move between opened files in editor (ctrl+tab) show list of opened editors
- [x] Highlighting 

## Writer plugin

- [ ] Simple word frequency analysis

## Feedback
- [ ] Make sure build command works properly, add settings
- [ ] Ways to handle settings
- [x] Add searchable command pallette to make commands runnable (ctrl-p)
- [ ] CMakeLists.txt html and xml-file formatting
- [ ] CMakeLists.txt html and xml-file code highlighting
- [x] Select all (ctrl-a)
- [x] Case insensitive matching in file list and command palette.
- [x] Exit to normal mode when saving.
- [ ] When alt+tab into editor, prevent from typing tab, also when closing other
      windows with ctrl + w
- [ ] Option to restore selection (gv)
- [x] Status line for common operations (for example saving, changing mode)
- [ ] Test: Add margin from the cursor to the bottom of the screen (when not on end of file)
- [ ] Fast way to create matching source for header (interaction with multiple alternative paths)
- [ ] Reload file lists when files are added or deleted (using the inotify code)
- [x] Replace selection instead of just inserting when pasting
- [ ] Do something about selection when undoing
- [x] When doing dd in beginning of file nothing happens, also dd moves cursor
      to previous row. That should not happend either
- [x] Highlighting does not update with timer (only when pressing key)
- [ ] Really want "rename_file" functionality
- [x] Also really want that the locator updates when there is changes to files
- [ ] Scroll-bar to show location in file
- [ ] Sometimes selection is active when not in visual mode. For example when
      pasing (maybe when pasting in visual mode or normal mode)
      - [ ] Fix that
      - [ ] Make so that changing mode and pressing escape always resets selection
- [ ] When trying to build and the build directory does not exist, it does not
      continue. Ask if the user wants to create the directory
- [ ] .medit.json does not reload when changed. Fix that
- [ ] Maybe sessions that reopens files when project is closed
- [x] Build before running
- [ ] Locator performance is really bad when opening large project
- [ ] Make ctrl+c work again
- [ ] Autocreate settings file with default settings to help user
- [ ] When using git blame. Copy the formatting from the original document to the blamed document
- [x] Create interaction for open previous projects
    - [ ] Save information about window layout, zoom and other things in the project. The project should be saved somewhere in the home folder.
- [x] "List-tool" that creates lists, then checklists and then checks the list (ctrl+l or just space in normal mode in markdown files)
- [ ] List issues in current document/on current line in console so they are more readable
- [ ] Include all files that is recognized by git
- [ ] Interaction for listing and opening any buffer that is loaded into memory
    - [ ] Links between interactions with  `[[]]`  syntax
- [ ] Add checkbocks asking if you want to push in commit interaction
      
## Bugs
- [x] When open a file where there is no project/.git, the program freezes
- [ ] Segmentation fault when using goto symbol on std::numeric::pi
- [x] Move cursor to the left when exiting input mode (like regular vim)
- [ ] When creating a new file and saving it, it does not show up in the quick open bar
- [ ] When scrolling horizontally, selection is not moved like it should
- [ ] Back history does not update when file is changed. Maybe make some way to update the cursor from revisions that could also be used for networking.


- [ ] Mark where on the line a annotation from lsp occucrs
- [ ] Option to show annotiation in tab below editor
    - [ ] When showing it center annotation from the current line
- [ ] Quicklist, consider using inheritance instead of function pointers for clarity
