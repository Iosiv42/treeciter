# Not Another AI slop-reverse-engineered software
Tree C Iter. Tree-like command utility clone.

## Building
Build using any tool you have:

```bash
cc treeciter.c -otreeciter
```

## Usage
Options:
```
-a  Also list files starting with "."
-L  Set depth limit (>= 1, default is no limit)
```

Example:
```bash
./treeciter -L2 -a ~
```
