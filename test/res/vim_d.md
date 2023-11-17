# Vim test


## dw

```
hello [N-dw]there you
-----------
hello [N]you
```


Special cases on line breaks
```
hello [N-dw]there
other
----
hello[N] 
other
```

```
hello [N-2dw]there
other
----
hello[N] 
```


## di(

```
int main([N-di(]x, y) {
}
----
int main([N]) {
}
```

## di" - inside

```
auto x = "hello[N-di"] there!";
----
auto x = "[N]";
```

## TODO: di" - outside

```
auto[N-di"] x = "hello there!";
----
auto x = "[N]";
```

## dtx

```
[N-dto]hello there
----
[N]o there
```

## dfx

```
[N-dfo]hello there
----
[N] there
```


## TODO: %

```
hello [N-d%](there y)ou
-----------
hello [N]ou
```

## D

```
hello [N-D]there you
----
hello[N] 
```

```
hello [N-2D]there you
another line that should be removed
----
hello[N] 
```


## x
```
hell[N-x]o
----
hel[N]l
```

## X - captial x
```
hell[N-X]o
----
hel[N]o
```

