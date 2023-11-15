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
