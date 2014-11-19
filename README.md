libbdiff
========

`libbdiff` is a simple C library for creating binary diffs and patching programs.

It uses Colin Percival's bytewise subtraction diffing algorithm, which can produce size-efficient binary diffs independent of system architecture.

## Building

### Dependencies
`libbdiff` requires `libbz2` to compress its diffs. 
On Debian-based systems this can be installed as follows:
```bash
sudo apt-get install libbz2-dev
```

`libbdiff` (should) build on all POSIX-compliant systems, including Linux, BSD, OS X, and Solaris.

Just do the following:

```bash
$ git clone https://github.com/woodruffw/libbdiff.git
$ cd libbdiff
$ make
```

This will leave `libbdiff.a`, a static library, in the repository root.

## Use and linking

For developing with `libbdiff`, see the [documenation](./docs/user_docs.md).

Build commands for `libbdiff` should look something like this:

```bash
$ gcc -Ipath/to/libbdiff.h file.c -o file -Lpath/to/libbdiff.a -lbdiff -lbz2
```

The `-I` and `-L` directives aren't required if `libbdiff.h` and `libbdiff.a` are installed to the system header and library directories properly.

## License

`libbdiff` uses algorithms and implementations from `bsdiff` and `bspatch`, and as such is licensed under the same BSD-style license.


