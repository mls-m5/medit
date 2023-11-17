# insert mode

## close {}

```
    hello {
        [I-}]
-----------
    hello {
    }[I]
```

## TODO open {}

```
    hello [N-o]{
-----------
    hello {
        [I]
    }
```

## TODO split {}

This is with a end brace

```
    hello {[I-\n]}
-----------
    hello {
        [I]
    }
```

## TODO newline after {

This is _without_ a end brace

```
    hello {[I-\n]
-----------
    hello {
        [I]
    }
```

## o

```
[N-o]hello
---
hello
[I]
```


## Exit insert -> movement

```
hel[I-<esc>]lo
---
he[N]llo
```

## TODO: I should move to after indentation

```
    hel[N-I]lo
---
    [I]hello
```

## Backspace on newline in insert mode

```
hello
[I-\b]
----
hello[I]
```


## TODO: Restore cursor position after undo
... Check vim for how it works
