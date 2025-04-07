Utilities for creating files of a specific size filled with zeros.

* `zero`: Creates a file containing zeros, writing in blocks. Useful for
          basic filesystem/disk write performance testing.
* `trunc`: Creates or truncates a file to a specific size using `ftruncate`.
           This is usually very fast as it doesn't write data, potentially
           creating a sparse file.

BUILD:

```
$ make
```

RUN `zero`:

```
$ ./zero [OPTIONS] FILENAME
```

OPTIONS for `zero`:

* -b SIZE: Size of each write block (default: 1G).
* -t SIZE : Total size of the file to create (default: block_size).
* -h      : Show help/usage.

SIZE format: _integer_ _suffix_ eg 10M, 2G, 1T.
Suffix is one of K, M, G, T or P (case-insensitive) and represent
powers of 2 so for example K=1024 (2^10) and M=1024*1024 (2^20)

Example: Create a 10GB file named `output.bin` by writing 1GB blocks:

```
$ ./zero -t 10G -b 1G output.bin
```

RUN `trunc`:

```
$ ./trunc FILENAME SIZE
```

Example: Create or truncate 'sparse.img' to exactly 1 Terabyte:

```
$ ./trunc sparse.img 1T
```
