# Vim test


## Syntax test 1

```
hello [N-dw]there you
-------------
hello [N]you
```

## dw

```
hello [N-dw]there you
==============
hello [N]you
```

## TODO: dw on end of line does not seem to work

```
hello [N-dw]there
----
hello [N]
```



## D

```
hello [N-D]there you
----
hello [N]
```

```
hello [N-2D]there you
another line that should be removed
----
hello [N]
```



