# Vim motions


## Simple move

```
hell[N-h]o
hel
-----------
hel[N]lo
hel
```

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


## Override preserved position when moving something else than vertical

```
hell[N-jh]o
hel
-----------
hello
h[N]el
```
