/**
 * 
 */

#include "bdiff.h"
#include "bpatch.h"

int lbd_diff(const char *oldf, const char *newf, const char *patchf)
{
	return lbd_diff_internal(oldf, newf, patchf);
}

int lbd_patch(const char *oldf, const char *patchf,
					const char *newf)
{
	return lbd_patch_internal(oldf, patchf, newf);
}