# Zero progress output and fast zero-fill

Design spec for `zero` and `trunc` utilities.

## Chunk size (`-b`)

**Chunk size (`-b`)** is the step size for progress updates and for chunked
I/O (fallocate zero-range on Linux, write-loop fallback). It does not change
the final file size — that is `-t` (total size). If `-t` is omitted, total
defaults to one chunk (1 GiB by default).

## Zero-fill strategy (physical zeros)

Files must contain **real zero bytes**, not sparse holes. Sparse `ftruncate`
is not used for non-empty files.

| Platform | Primary | Fallback |
|----------|---------|----------|
| Linux | `fallocate(FALLOC_FL_ZERO_RANGE)` | mmap + `write()` loop |
| macOS / BSD | mmap + `write()` loop | — |

After fill, `fstat` verifies `st_size` matches the requested total.

`trunc` is a legacy wrapper calling the same `zerofill()` core.

## Size units: binary (2^n)

Powers of 1024 throughout — not SI decimal.

| Suffix | Multiplier |
|--------|------------|
| (none) | 1 byte |
| K/k | 2^10 |
| M/m | 2^20 |
| G/g | 2^30 |
| T/t | 2^40 |
| P/p | 2^50 |

Display uses IEC labels: KiB, MiB, GiB, TiB, PiB.

## Progress output

- TTY: single `\r`-updated line with percent, done/total, rate, ETA
- Non-TTY: periodic newline updates
- `-q`: suppress progress and startup method line; summary still prints
- Final summary always includes fill method used

## Method reporting

Startup (unless `-q`):

```
zero: filling big.bin via fallocate zero-range
```

With fallback:

```
zero: filling big.bin via write loop
zero: note: fallocate zero-range unavailable on this filesystem
```

Summary (always):

```
zero: wrote big.bin  10.00 GiB in 8.3s  (1.20 GiB/s avg, fallocate zero-range)
```

## Exit codes

- 0: success
- 1: runtime error
- 2: usage error

## CLI

```
zero [OPTIONS] FILENAME
  -b SIZE   chunk size (default: 1G = 1 GiB)
  -t SIZE   total size (default: same as -b)
  -q        quiet
  -h        help
  -V        version
```
