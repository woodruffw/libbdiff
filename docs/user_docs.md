libbdiff user documentation
===========================

## Headers

To use `libbdiff`, you only need to include a single header file:

```C
#include <libbdiff.h>
```

If `libbdiff.h` isn't in a system headers directory, make sure to use
a relative include instead.

## Functions

Diffing and patching binaries with `libbdiff` only requires you to interact
with two functions:

### `int lbd_diff(const char *oldf, const char *newf, const char *patchf)`

Creates a binary diff file of name `patchf` from `oldf` and `newf`.
Both `oldf` and `newf` **must** exist on the filesystem.
Any extant file `patchf` will be **overwritten**.

#### Return
On success, `lbd_diff` returns `LBD_OK`.
On failure, `lbd_diff` returns one of the error codes [below](#errorcodes).

### `int lbd_patch(const char *oldf, const char *patchf, const char *newf)`

Creates a new, patched file of name `newf` from `oldf` and `patchf`.
Both `oldf` and `patchf` **must** exist on the filesystem.
Any extant file `newf` will be **overwritten**.

#### Return
On success, `lbd_patch` returns `LBD_OK`.
On failure, `lbd_patch` returns one of the error codes [below](#errorcodes).

## Error codes

`libbdiff` defines the following error and status codes:

- `LBD_OK` - An 'okay' status. Functions that return `LBD_OK` did not experience an error.
- `LBD_ERR_OPEN` - An error occurred while opening a file for reading or writing. Make sure that any file argument(s) have the proper permissions and exist.
- `LBD_ERR_CLOSE` - An error occurred while closing a file. Diffing large binaries on a resource-constrained system may cause this.
- `LBD_ERR_SEEK` - A file seek error occurred.
- `LBD_ERR_TELL` - A file tell error occurred.
- `LBD_ERR_READ` - A file read error occurred.
- `LBD_ERR_WRITE` - A file write error occurred.
- `LBD_ERR_CORRUPT` - `lbd_patch` only. The provided patch file is corrupted or lacks a header.
- `LBD_ERR_BZ` - A libbzip2 error occurred. 