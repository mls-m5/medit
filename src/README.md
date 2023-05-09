

# Cases

## Rename interaction 2023-05-07

- [x] Create rename standard command
- [x] Interaction serialization
- [x] Create new operation buffers
   - [x] (should it support multiple users interacitons?) - no not in the beginning
      - It could mean that multiple users could do renaming in multiple files at
        the same time causing conflicts (this kind of operation should probably
        not be permitted
      - If the changes is only related to one file it might be okay
- [ ] Open operation buffer in some kind of bottom view that is vissible at the same time as
      the current editor
- [x] Refactor how plugins work
   - [x] Get plugin action related to current buffer from environment instead of window
- [x] Generate text file with the changes to be made into operation buffer
- [x] Apply the changes in the operation buffer with return, abort with ctrl+w
- [ ] ?Make the operation buffers specify special characters that can be used to 
      accept or reject so that any keys kan be assigned.
      This way the interactions can be used for even save-prompts and other stuff
- [x] Handle errors when renaming is not possible
- [ ] Redraw screen when buffer is opened or closed
- [ ] Update highlighting on emitted changes instead of keypress
    - [ ] Move highlighting handling into core environment
- [ ] Unload files after applying changes to closed files

## Save interaction 2023-05-09
- [ ] Send message to lsp server when buffer is saved


