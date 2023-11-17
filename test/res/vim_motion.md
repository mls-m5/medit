# Vim motions

## %

```
hello [N-%](there y)ou
-----------
hello (there y[N])ou
```

## TODO: % outside

```
[N-%]hello (there y)ou
-----------
hello (there y[N])ou
```



## Normal mod cursor should not move past last character

```
hell[N-l]o
-----------
hell[N]o
```

## Preserve cursor position between lines

```
hell[N-jj]o

hello
-----------
hello

hell[N]o
```
