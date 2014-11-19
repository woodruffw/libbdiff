/**
 * bpatch.c
 * Functions used by libbdiff internally to patch binary files.
 *
 * Copyright 2003-2005 Colin Percival
 * Copyright 2014 William Woodruff (william @ tuffbizz.com)
 * All rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted providing that the following conditions 
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdint.h>
#include <bzlib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "libbdiff_status.h"

static off_t lbd_offtin(uint8_t *buf)
{
	off_t y;

	y = buf[7] & 0x7F;
	y = y * 256;
	y += buf[6];
	y = y * 256;
	y += buf[5];
	y = y * 256;
	y += buf[4];
	y = y * 256;
	y += buf[3];
	y = y * 256;
	y += buf[2];
	y = y * 256;
	y += buf[1];
	y = y * 256;
	y += buf[0];

	if (buf[7] & 0x80)
		y = -y;

	return y;
}

int lbd_patch_internal(const char *oldf, const char *patchf,
	const char *newf)
{
	FILE *f, *cpf, *dpf, *epf;
	BZFILE *cpfbz2, *dpfbz2, *epfbz2;
	int cbz2err, dbz2err, ebz2err;
	int fd;
	ssize_t oldsize, newsize;
	ssize_t bzctrllen, bzdatalen;
	uint8_t header[32], buf[8];
	uint8_t *old, *new;
	off_t oldpos, newpos;
	off_t ctrl[3];
	off_t lenread;
	off_t i;

	if ((f = fopen(patchf, "r")) == NULL)
	{
		return LBD_ERR_OPEN;
	}

	/*
	File format:
		0	8	"LBDIFFXX"
		8	8	X
		16	8	Y
		24	8	sizeof(newfile)
		32	X	bzip2(control block)
		32+X	Y	bzip2(diff block)
		32+X+Y	???	bzip2(extra block)
	with control block a set of triples (x,y,z) meaning "add x bytes
	from oldfile to x bytes from the diff block; copy y bytes from the
	extra block; seek forwards in oldfile by z bytes".
	*/

	/* Read header */
	if (fread(header, 1, 32, f) < 32)
	{
		if (feof(f))
		{
			return LBD_ERR_CORRUPT;
		}
		
		return LBD_ERR_READ;
	}

	/* Check for appropriate magic */
	if (memcmp(header, "LBDIFFXX", 8) != 0)
	{
		return LBD_ERR_CORRUPT;
	}

	/* Read lengths from header */
	bzctrllen = lbd_offtin(header + 8);
	bzdatalen = lbd_offtin(header + 16);
	newsize = lbd_offtin(header + 24);
	if ((bzctrllen < 0) || (bzdatalen < 0) || (newsize < 0))
	{
		return LBD_ERR_CORRUPT;
	}

	/* Close patch file and re-open it via libbzip2 at the right places */
	if (fclose(f))
	{
		return LBD_ERR_CLOSE;
	}

	if ((cpf = fopen(patchf, "r")) == NULL)
	{
		return LBD_ERR_OPEN;
	}

	if (fseeko(cpf, 32, SEEK_SET))
	{
		return LBD_ERR_SEEK;
	}

	if ((cpfbz2 = BZ2_bzReadOpen(&cbz2err, cpf, 0, 0, NULL, 0)) == NULL)
	{
		return LBD_ERR_BZ;
	}

	if ((dpf = fopen(patchf, "r")) == NULL)
	{
		return LBD_ERR_OPEN;
	}

	if (fseeko(dpf, 32 + bzctrllen, SEEK_SET))
	{
		return LBD_ERR_SEEK;
	}

	if ((dpfbz2 = BZ2_bzReadOpen(&dbz2err, dpf, 0, 0, NULL, 0)) == NULL)
	{
		return LBD_ERR_BZ;
	}

	if ((epf = fopen(patchf, "r")) == NULL)
	{
		return LBD_ERR_OPEN;
	}

	if (fseeko(epf, 32 + bzctrllen + bzdatalen, SEEK_SET))
	{
		return LBD_ERR_SEEK;
	}
	
	if ((epfbz2 = BZ2_bzReadOpen(&ebz2err, epf, 0, 0, NULL, 0)) == NULL)
	{
		return LBD_ERR_BZ;
	}

	if (((fd = open(oldf, O_RDONLY, 0)) < 0) ||
		((oldsize = lseek(fd, 0, SEEK_END)) == -1) ||
		((old = malloc(oldsize + 1)) == NULL) ||
		(lseek(fd, 0, SEEK_SET) != 0) ||
		(read(fd, old, oldsize) != oldsize) ||
		(close(fd) == -1))
	{
		return LBD_ERR_OPEN;
	}

	if ((new = malloc(newsize + 1)) == NULL)
		return LBD_ERR_MALLOC;

	oldpos = 0;
	newpos = 0;
	while (newpos < newsize)
	{
		/* Read control data */
		for (i = 0;i <= 2; i++)
		{
			lenread = BZ2_bzRead(&cbz2err, cpfbz2, buf, 8);

			if ((lenread < 8) || ((cbz2err != BZ_OK) &&
			    (cbz2err != BZ_STREAM_END)))
			{
				return LBD_ERR_CORRUPT;
			}

			ctrl[i] = lbd_offtin(buf);
		}

		/* Sanity-check */
		if (newpos + ctrl[0] > newsize)
		{
			return LBD_ERR_CORRUPT;
		}

		/* Read diff string */
		lenread = BZ2_bzRead(&dbz2err, dpfbz2, new + newpos, ctrl[0]);
		if ((lenread < ctrl[0]) ||
		    ((dbz2err != BZ_OK) && (dbz2err != BZ_STREAM_END)))
		{
			return LBD_ERR_CORRUPT;
		}

		/* Add old data to diff string */
		for (i=0; i < ctrl[0]; i++)
			if ((oldpos + i >= 0) && (oldpos + i < oldsize))
				new[newpos + i] += old[oldpos + i];

		/* Adjust pointers */
		newpos += ctrl[0];
		oldpos += ctrl[0];

		/* Sanity-check */
		if (newpos + ctrl[1] > newsize)
		{
			return LBD_ERR_CORRUPT;
		}

		/* Read extra string */
		lenread = BZ2_bzRead(&ebz2err, epfbz2, new + newpos, ctrl[1]);
		if ((lenread < ctrl[1]) ||
		    ((ebz2err != BZ_OK) && (ebz2err != BZ_STREAM_END)))
		{
			return LBD_ERR_CORRUPT;
		}

		/* Adjust pointers */
		newpos += ctrl[1];
		oldpos += ctrl[2];
	}

	/* Clean up the bzip2 reads */
	BZ2_bzReadClose(&cbz2err, cpfbz2);
	BZ2_bzReadClose(&dbz2err, dpfbz2);
	BZ2_bzReadClose(&ebz2err, epfbz2);
	if (fclose(cpf) || fclose(dpf) || fclose(epf))
	{
		return LBD_ERR_CLOSE;
	}

	/* Write the new file */
	if (((fd = open(newf, O_CREAT|O_TRUNC|O_WRONLY, 0666)) < 0) ||
		(write(fd, new, newsize) != newsize) || (close(fd) == -1))
	{
		return LBD_ERR_OPEN;
	}

	free(new);
	free(old);

	return LBD_OK;
}