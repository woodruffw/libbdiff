/**
 * 
 */

#include <stdio.h>
#include <stdint.h>
#include <bzlib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libbdiff_status.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

static void lbd_split(off_t *I, 
	off_t *V, off_t start, off_t len, off_t h)
{
	off_t i, j, k, x, tmp, jj, kk;

	if (len < 16)
	{
		for (k = start; k < (start + len); k += j)
		{
			j = 1;
			x = V[I[k] + h];

			for (i = 1; (k + i) < (start + len); i++)
			{
				if (V[I[k + i] + h] < x)
				{
					x = V[I[k + i] + h];
					j = 0;
				}
				if (V[I[k + i] + h] == x)
				{
					tmp = I[k + j];
					I[k + j] = I[k + i];
					I[k + i] = tmp;
					j++;
				}
			}

			for (i = 0; i < j; i++)
				V[I[k + i]] = k + j - 1;
			if (j == 1)
				I[k] = -1;
		}

		return;
	}

	x = V[I[start + len / 2] + h];
	jj = 0;
	kk = 0;

	for (i = start; i < start + len; i++)
	{
		if (V[I[i] + h] < x)
			jj++;
		if (V[I[i] + h] == x)
			kk++;
	}

	jj += start;
	kk += jj;

	i = start;
	j = 0;
	k = 0;

	while (i < jj)
	{
		if (V[I[i] + h] < x)
		{
			i++;
		} 
		else if (V[I[i] + h] == x)
		{
			tmp = I[i];
			I[i] = I[jj + j];
			I[jj + j] = tmp;
			j++;
		}
		else
		{
			tmp = I[i];
			I[i] = I[kk + k];
			I[kk+k] = tmp;
			k++;
		}
	}

	while (jj + j < kk)
	{
		if (V[I[jj + j] + h] == x)
		{
			j++;
		}
		else
		{
			tmp = I[jj + j];
			I[jj + j] = I[kk + k];
			I[kk + k] = tmp;
			k++;
		}
	}

	if (jj > start)
		lbd_split(I, V, start, (jj - start), h);

	for (i=0; i < (kk-jj); i++)
		V[I[jj + i]] = kk - 1;

	if (jj == kk - 1)
		I[jj] = - 1;

	if (start + len > kk)
		lbd_split(I, V, kk, start + len - kk, h);
}

static void lbd_qsufsort(off_t *I,
	off_t *V, uint8_t *old, off_t oldsize)
{
	off_t buckets[256];
	off_t i, h, len;

	for (i = 0; i < 256; i++)
		buckets[i] = 0;

	for (i = 0;i < oldsize; i++)
		buckets[old[i]]++;

	for (i = 1;i < 256; i++)
		buckets[i] += buckets[i - 1];

	for (i = 255; i > 0; i--)
		buckets[i] = buckets[i - 1];

	buckets[0] = 0;

	for (i = 0; i < oldsize; i++)
		I[++buckets[old[i]]] = i;

	I[0] = oldsize;

	for (i = 0; i < oldsize; i++)
		V[i] = buckets[old[i]];

	V[oldsize] = 0;

	for (i = 1; i < 256; i++)
	{
		if (buckets[i] == buckets[i - 1] + 1)
			I[buckets[i]] = -1;
	}

	I[0] = -1;

	for (h = 1; I[0] != -(oldsize + 1); h += h)
	{
		len = 0;

		for (i = 0; i < oldsize + 1;)
		{
			if (I[i] < 0)
			{
				len -= I[i];
				i -= I[i];
			}
			else
			{
				if (len)
					I[i - len] = -len;

				len = V[I[i]] + 1 - i;
				lbd_split(I, V, i, len, h);
				i += len;
				len = 0;
			}
		}

		if (len)
			I[i - len] = -len;
	}

	for (i = 0; i < oldsize + 1; i++)
		I[V[i]] = i;
}

static off_t lbd_matchlen
	(uint8_t *old, off_t oldsize, uint8_t *new, off_t newsize)
{
	off_t i;

	for (i = 0; (i < oldsize) && (i < newsize); i++)
	{
		if (old[i] != new[i])
			break;
	}

	return i;
}

static off_t lbd_search(off_t *I,
	uint8_t *old, off_t oldsize, uint8_t *new, off_t newsize, off_t st,
	off_t en, off_t *pos)
{
	off_t x, y;

	if ((en - st) < 2)
	{
		x = lbd_matchlen(old + I[st], oldsize - I[st], new, newsize);
		y = lbd_matchlen(old + I[en], oldsize - I[en], new, newsize);

		if (x > y)
		{
			*pos = I[st];
			return x;
		}
		else
		{
			*pos = I[en];
			return y;
		}
	}

	x = st + (en - st) / 2;
	if (memcmp(old + I[x], new, MIN(oldsize - I[x], newsize)) < 0)
	{
		return lbd_search(I, old, oldsize, new, newsize, x, en, pos);
	}
	else
	{
		return lbd_search(I, old, oldsize, new, newsize, st, x, pos);
	}
}

static void lbd_offtout(off_t x, uint8_t *buf)
{
	off_t y;

	if (x < 0)
		y = -x; 
	else
		y = x;

	buf[0] = y % 256;
	y -= buf[0];
	y = y / 256;
	buf[1] = y % 256;
	y -= buf[1];
	y = y / 256;
	buf [2] = y % 256;
	y -= buf[2];
	y = y/256;
	buf[3] = y % 256;
	y -= buf[3];
	y = y / 256;
	buf[4] = y % 256;
	y -= buf[4];
	y= y / 256;
	buf[5] = y % 256;
	y -= buf[5];
	y = y / 256;
	buf[6] = y % 256;
	y -= buf[6];
	y = y / 256;
	buf[7] = y % 256;


	if (x < 0)
		buf[7] |= 0x80;
}

int lbd_diff_internal(const char *oldf, const char *newf,
	const char *patchf)
{
	int fd;
	uint8_t *old, *new;
	off_t oldsize, newsize;
	off_t *I, *V;
	off_t scan, pos, len;
	off_t lastscan, lastpos, lastoffset;
	off_t oldscore, scsc;
	off_t s, Sf, lenf, Sb, lenb;
	off_t overlap, Ss, lens;
	off_t i;
	off_t dblen, eblen;
	uint8_t *db, *eb;
	uint8_t buf[8];
	uint8_t header[32];
	FILE *pf;
	BZFILE *pfbz2;
	int bz2err;

	/* Allocate oldsize+1 bytes instead of oldsize bytes to ensure
		that we never try to malloc(0) and get a NULL pointer */
	if (((fd = open(oldf, O_RDONLY, 0)) < 0) ||
		((oldsize = lseek(fd, 0, SEEK_END)) == -1) ||
		((old = malloc(oldsize + 1)) == NULL) ||
		(lseek(fd, 0, SEEK_SET) != 0) ||
		(read(fd, old, oldsize) != oldsize) ||
		(close(fd) == -1))
	{
		return LBD_ERR_OPEN;
	}

	if (((I = malloc((oldsize + 1) * sizeof(off_t))) == NULL) ||
		((V = malloc((oldsize + 1) * sizeof(off_t))) == NULL))
	{
		return LBD_ERR_MALLOC;
	}

	lbd_qsufsort(I, V, old, oldsize);

	free(V);

	/* Allocate newsize+1 bytes instead of newsize bytes to ensure
		that we never try to malloc(0) and get a NULL pointer */
	if (((fd = open(newf, O_RDONLY, 0)) < 0) ||
		((newsize = lseek(fd, 0, SEEK_END)) == -1) ||
		((new = malloc(newsize + 1)) == NULL) ||
		(lseek(fd, 0, SEEK_SET) !=0 ) ||
		(read(fd, new, newsize) != newsize) ||
		(close(fd) == -1))
	{
		return LBD_ERR_OPEN;
	}

	if (((db = malloc(newsize + 1)) == NULL) ||
		((eb = malloc(newsize + 1)) == NULL))
	{
		return LBD_ERR_MALLOC;
	}

	dblen=0;
	eblen=0;

	/* Create the patch file */
	if ((pf = fopen(patchf, "w")) == NULL)
	{
		return LBD_ERR_OPEN;
	}

	/* Header is
		0	8	 "LBDIFFXX"
		8	8	length of bzip2ed ctrl block
		16	8	length of bzip2ed diff block
		24	8	length of new file */
	/* File is
		0	32	Header
		32	??	Bzip2ed ctrl block
		??	??	Bzip2ed diff block
		??	??	Bzip2ed extra block */
	memcpy(header, "LBDIFFXX", 8);
	lbd_offtout(0, header + 8);
	lbd_offtout(0, header + 16);
	lbd_offtout(newsize, header + 24);

	if (fwrite(header, 32, 1, pf) != 1)
	{
		return LBD_ERR_WRITE;
	}

	/* Compute the differences, writing ctrl as we go */
	if ((pfbz2 = BZ2_bzWriteOpen(&bz2err, pf, 9, 0, 0)) == NULL)
	{
		return LBD_ERR_BZ;
	}

	scan = 0;
	len = 0;
	lastscan = 0;
	lastpos = 0;
	lastoffset = 0;

	while (scan < newsize)
	{
		oldscore = 0;

		for (scsc = (scan += len); scan < newsize; scan++)
		{
			len = lbd_search(I, old, oldsize, new + scan,
						newsize-scan, 0, oldsize, &pos);

			for (; scsc < scan+len; scsc++)
			{
				if ((scsc + lastoffset < oldsize) &&
					(old[scsc + lastoffset] == new[scsc]))
				{
					oldscore++;
				}
			}

			if (((len == oldscore) && (len != 0)) || 
				(len > oldscore + 8))
				break;

			if ((scan + lastoffset < oldsize) &&
				(old[scan + lastoffset] == new[scan]))
				oldscore--;
		}

		if ((len != oldscore) || (scan == newsize))
		{
			s = 0;
			Sf = 0;
			lenf = 0;

			for (i = 0; (lastscan + i < scan) && (lastpos + i < oldsize);)
			{
				if (old[lastpos + i] == new[lastscan + i])
					s++;
				
				i++;

				if (s * 2 - i > Sf * 2 - lenf)
				{
					Sf = s;
					lenf = i;
				}
			}

			lenb = 0;
			if (scan < newsize) {
				s=0;
				Sb=0;
				
				for (i=1; (scan >= lastscan + i) && (pos >= i); i++)
				{
					if (old[pos - i] == new[scan - i])
						s++;
					if (s * 2 - i > Sb * 2 - lenb)
					{
						Sb = s;
						lenb = i; 
					}
				}
			}

			if (lastscan + lenf > scan - lenb)
			{
				overlap = (lastscan + lenf) - (scan - lenb);
				s = 0;
				Ss = 0;
				lens = 0;

				for (i = 0; i < overlap; i++)
				{
					if (new[lastscan + lenf - overlap + i] 
						== old[lastpos + lenf - overlap + i])
					{
						s++;
					}

					if (new[scan - lenb + i] == old[pos - lenb + i]) 
						s--;

					if (s > Ss)
					{
						Ss = s;
						lens = i + 1;
					}
				}

				lenf += lens - overlap;
				lenb -= lens;
			}

			for (i = 0; i < lenf; i++)
				db[dblen + i] = new[lastscan + i] - old[lastpos + i];

			for (i = 0; i < (scan - lenb) - (lastscan + lenf); i++)
				eb[eblen + i] = new[lastscan + lenf + i];

			dblen += lenf;
			eblen += (scan - lenb) - (lastscan + lenf);

			lbd_offtout(lenf, buf);
			BZ2_bzWrite(&bz2err, pfbz2, buf, 8);

			if (bz2err != BZ_OK)
			{
				return LBD_ERR_BZ;
			}

			lbd_offtout((scan - lenb) - (lastscan + lenf), buf);
			BZ2_bzWrite(&bz2err, pfbz2, buf, 8);

			if (bz2err != BZ_OK)
			{
				return LBD_ERR_BZ;
			}

			lbd_offtout((pos - lenb) - (lastpos + lenf), buf);
			BZ2_bzWrite(&bz2err, pfbz2, buf, 8);

			if (bz2err != BZ_OK)
			{
				return LBD_ERR_BZ;
			}

			lastscan = scan - lenb;
			lastpos = pos - lenb;
			lastoffset = pos - scan;
		}
	}

	BZ2_bzWriteClose(&bz2err, pfbz2, 0, NULL, NULL);

	if (bz2err != BZ_OK)
	{
		return LBD_ERR_BZ;
	}

	/* Compute size of compressed ctrl data */
	if ((len = ftello(pf)) == -1)
	{
		return LBD_ERR_TELL;
	}

	lbd_offtout(len - 32, header + 8);

	/* Write compressed diff data */
	if ((pfbz2 = BZ2_bzWriteOpen(&bz2err, pf, 9, 0, 0)) == NULL)
	{
		return LBD_ERR_BZ;
	}

	BZ2_bzWrite(&bz2err, pfbz2, db, dblen);

	if (bz2err != BZ_OK)
	{
		return LBD_ERR_BZ;
	}

	BZ2_bzWriteClose(&bz2err, pfbz2, 0, NULL, NULL);

	if (bz2err != BZ_OK)
	{
		return LBD_ERR_BZ;
	}

	/* Compute size of compressed diff data */
	if ((newsize = ftello(pf)) == -1)
	{
		return LBD_ERR_TELL;
	}
	
	lbd_offtout(newsize - len, header + 16);

	/* Write compressed extra data */
	if ((pfbz2 = BZ2_bzWriteOpen(&bz2err, pf, 9, 0, 0)) == NULL)
	{
		return LBD_ERR_BZ;
	}

	BZ2_bzWrite(&bz2err, pfbz2, eb, eblen);

	if (bz2err != BZ_OK)
	{
		return LBD_ERR_BZ;
	}

	BZ2_bzWriteClose(&bz2err, pfbz2, 0, NULL, NULL);

	if (bz2err != BZ_OK)
	{
		return LBD_ERR_BZ;
	}

	/* Seek to the beginning, write the header, and close the file */
	if (fseeko(pf, 0, SEEK_SET))
	{
		return LBD_ERR_SEEK;
	}

	if (fwrite(header, 32, 1, pf) != 1)
	{
		return LBD_ERR_WRITE;
	}

	if (fclose(pf))
	{
		return LBD_ERR_CLOSE;
	}

	/* Free the memory we used */
	free(db);
	free(eb);
	free(I);
	free(old);
	free(new);

	return LBD_OK;
}