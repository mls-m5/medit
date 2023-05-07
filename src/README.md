

# Cases

## Rename interaction 2023-05-07

- [x] Create rename standard command
- [ ] Create new operation buffers
   - [ ] (should it support multiple users interacitons?)
      - It could mean that multiple users could do renaming in multiple files at
        the same time causing conflicts (this kind of operation should probably
        not be permitted
      - If the changes is only related to one file it might be okay
   - [ ] Open in some kind of bottom view that is vissible at the same time as
         the current editor
- [ ] Refactor how plugins work
   - [ ] Get plugin action related to current buffer from environment instead of window
- [ ] Generate text file with the changes to be made into operation buffer
- [ ] Apply the changes in the operation buffer with return, abort with esc
