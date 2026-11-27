# clangd-client

Clangd client implemented using linux pipes.

Clangd is started as a separate process, and communication is done through
stdin and stdout.


An example interchange between the client (this program) and the server (clangd)
can look like this:

Request:
```http
Content-Length: 210

{
    "id": 1,
    "jsonrpc": "2.0",
    "method": "initialize",
    "params": {
        "clientInfo": {
            "name": "clangd-client",
            "version": "0"
        },
        "processId": 0
    }
}
```

Notice how the message id of the response matches that of the request from the
server:
```http
Content-Length: 1734

{
  "id": 1,
  "jsonrpc": "2.0",
  "result": {
    "capabilities": {
      "astProvider": true,
      "callHierarchyProvider": true,
      "codeActionProvider": true,
      "compilationDatabase": {
        "automaticReload": true
      },
      "completionProvider": {
        "allCommitCharacters": [
          " ",
          "\t",
          "(",
          ")",
          "[",
          "]",
          "{",
          "}",
          "<",
          ">",
          ":",
          ";",
          ",",
          "+",
          "-",
          "/",
          "*",
          "%",
          "^",
          "&",
          "#",
          "?",
          ".",
          "=",
          "\"",
          "'",
          "|"
        ],
        "resolveProvider": false,
        "triggerCharacters": [
          ".",
          "<",
          ">",
          ":",
          "\"",
          "/"
        ]
      },
      "declarationProvider": true,
      "definitionProvider": true,
      "documentFormattingProvider": true,
      "documentHighlightProvider": true,
      "documentLinkProvider": {
        "resolveProvider": false
      },
      "documentOnTypeFormattingProvider": {
        "firstTriggerCharacter": "\n",
        "moreTriggerCharacter": []
      },
      "documentRangeFormattingProvider": true,
      "documentSymbolProvider": true,
      "executeCommandProvider": {
        "commands": [
          "clangd.applyFix",
          "clangd.applyTweak"
        ]
      },
      "hoverProvider": true,
      "implementationProvider": true,
      "memoryUsageProvider": true,
      "referencesProvider": true,
      "renameProvider": true,
      "selectionRangeProvider": true,
      "semanticTokensProvider": {
        "full": {
          "delta": true
        },
        "legend": {
          "tokenModifiers": [
            "declaration",
            "deprecated",
            "deduced",
            "readonly",
            "static",
            "abstract",
            "dependentName",
            "defaultLibrary",
            "functionScope",
            "classScope",
            "fileScope",
            "globalScope"
          ],
          "tokenTypes": [
            "variable",
            "variable",
            "parameter",
            "function",
            "method",
            "function",
            "property",
            "variable",
            "class",
            "interface",
            "enum",
            "enumMember",
            "type",
            "type",
            "unknown",
            "namespace",
            "typeParameter",
            "concept",
            "type",
            "macro",
            "comment"
          ]
        },
        "range": false
      },
      "signatureHelpProvider": {
        "triggerCharacters": [
          "(",
          ","
        ]
      },
      "textDocumentSync": {
        "change": 2,
        "openClose": true,
        "save": true
      },
      "typeHierarchyProvider": true,
      "workspaceSymbolProvider": true
    },
    "serverInfo": {
      "name": "clangd",
      "version": "Ubuntu clangd version 13.0.1-2ubuntu2.1 linux+grpc x86_64-pc-linux-gnu"
    }
  }
}
```


## Licence

Just slapped on a gnu licence.

## Include flags

By default, clangd does not seem to find any include files at all, not even
the standard includes.

https://github.com/clangd/clangd/issues/617

This should work in theory, but in practice does not work
```
export CLANGD_FLAGS="-I/usr/include/c++/11 --log=Verbose"
```

Another possibility is to use query-drivers
https://releases.llvm.org/10.0.0/tools/clang/tools/extra/docs/clangd/Configuration.html

Did not work either. But I found a command that clang seems to be using
internally when running with verbosity on

It seems like that the only working solution is to edit `~/.config/clangd/config.yaml`

Run `g++ -E -xc++ -v /dev/null` and get all paths to all standard include stuff

Then add (replace with what includes you found)
```
CompileFlags:
  Add: [-I/usr/include/c++/11, -I/usr/include/x86_64-linux-gnu/c++/11, -I/usr/include/c++/11/backward, -I/usr/lib/gcc/x86_64-linux-gnu/11/include, -I/usr/local/include, -I/usr/include/x86_64-linux-gnu, -I/usr/include]
```

to `~/.config/clangd/config.yaml` or `.clangd` inside your project directiory.
You could also add project specific include paths there.


Another possibility is to use environment variables like so:

```
CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/x86_64-linux-gnu/c++/11:/usr/include/c++/11/backward:/usr/lib/gcc/x86_64-linux-gnu/11/include:/usr/local/include:/usr/include/x86_64-linux-gnu -I/usr/include
```

## References
https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/
