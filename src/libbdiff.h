/**
 * 
 */

#ifndef LIBBDIFF_H
#define LIBBDIFF_H

#include "libbdiff_status.h"

int lbd_diff(const char *oldf, const char *newf, const char *patchf);
int lbd_patch(const char *oldf, const char *patchf, const char *newf);

#endif /* LIBBDIFF_H */