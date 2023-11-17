# Vim test - c

## cw

```
hello [N-cw]there
----
hello[I] 
```

This is tricky since it's differs from dw in vim.

```
hello [N-cw]there you
----
hello [I] you
```

## ctx

```
[N-cto]hello there
----
[I]o there
```

## cfx

```
[N-cfo]hello there
----
[I] there
```

## C

```
hello [N-C]there you
----
hello [I]
```

```
hello [N-2C]there you
another line that should be removed
----
hello [I]
```
