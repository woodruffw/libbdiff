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
$ sudo make install # installs libbdiff.a and the headers to PREFIX
```

This will leave `libbdiff.a`, a static library, in the repository root.

## Use and linking

For developing with `libbdiff`, see the [documenation](./docs/user_docs.md).

Build commands for `libbdiff` should look something like this:

```bash
$ gcc -Ipath/to/libbdiff.h file.c -o file -Lpath/to/libbdiff.a -lbdiff
```

The `-I` and `-L` directives aren't required if `libbdiff.h` and `libbdiff.a` are installed to system header and library directories.

## License

`libbdiff` uses algorithms and implementations from `bsdiff` and `bspatch`, and as such is licensed under the same BSD-style license.
It also uses and links with `libbz2`, which is also licensed under a BSD-style license.

### `bsdiff`/`bspatch`
> Copyright 2003-2005 Colin Percival
>
> Redistribution and use in source and binary forms, with or without
> modification, are permitted providing that the following conditions 
> are met:
> 1. Redistributions of source code must retain the above copyright
>    notice, this list of conditions and the following disclaimer.
> 2. Redistributions in binary form must reproduce the above copyright
>    notice, this list of conditions and the following disclaimer in the
>    documentation and/or other materials provided with the distribution.
>
> THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
> IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
> WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
> ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
> DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
> DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
> OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
> HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
> STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
> IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
> POSSIBILITY OF SUCH DAMAGE.

### `libbz2`
> This program, "bzip2", the associated library "libbzip2", and all
> documentation, are copyright (C) 1996-2010 Julian R Seward.  All
> rights reserved.
> 
> Redistribution and use in source and binary forms, with or without
> modification, are permitted provided that the following conditions
> are met:
> 
> 1. Redistributions of source code must retain the above copyright
>    notice, this list of conditions and the following disclaimer.
> 
> 2. The origin of this software must not be misrepresented; you must 
>    not claim that you wrote the original software.  If you use this 
>    software in a product, an acknowledgment in the product 
>    documentation would be appreciated but is not required.
> 
> 3. Altered source versions must be plainly marked as such, and must
>    not be misrepresented as being the original software.
> 
> 4. The name of the author may not be used to endorse or promote 
>    products derived from this software without specific prior written 
>    permission.
> 
> THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
> OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
> WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
> ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
> DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
> DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
> GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
> INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
> WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
> NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
> SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
> 
> Julian Seward, jseward@bzip.org
> bzip2/libbzip2 version 1.0.6 of 6 September 2014
