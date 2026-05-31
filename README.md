Utilities for creating files of a specific size filled with zeros.

* `zero`: Creates a file with **physical zeros** written to disk. On Linux it
          uses `fallocate` zero-range when available; otherwise (and on other
          OSes) it writes zero bytes via a mmap buffer loop.
* `trunc`: Legacy wrapper with `FILE SIZE` argument order. Same zero-fill
           behavior as `zero`. Prefer `zero -t SIZE FILENAME`.

See [specs/zero-progress-and-speed.md](specs/zero-progress-and-speed.md) for
design details.

BUILD:

```
$ make
```

RUN `zero`:

```
$ ./zero [OPTIONS] FILENAME
```

OPTIONS for `zero`:

* `-b SIZE`: Chunk size for progress updates (default: 1G = 1 GiB).
* `-t SIZE`: Total file size (default: same as `-b`).
* `-q`: Quiet; suppress progress (summary still printed).
* `-h`: Show help.
* `-V`: Show version.

SIZE format: _integer_ [_suffix_] eg `10M`, `2G`, `1T`.
Suffix is one of K, M, G, T or P (case-insensitive). Suffixes are **powers of
1024** (binary/IEC), not decimal: K=1024, M=1024², G=1024³, T=1024⁴, P=1024⁵.
Bare integer means bytes.

Examples:

```
$ ./zero -t 10G output.bin
$ ./zero -t 1T big.img
$ ./zero -t 10G -b 1G output.bin
```

Progress output uses IEC units (KiB, MiB, GiB, …) and reports which zero-fill
method was used.

RUN `trunc`:

```
$ ./trunc FILENAME SIZE
```

Example:

```
$ ./trunc sparse.img 1T
```

Prefer: `./zero -t 1T sparse.img`

## License

MIT License. Copyright (c) 2009-2026 David Beckett. See [LICENSE](LICENSE).
