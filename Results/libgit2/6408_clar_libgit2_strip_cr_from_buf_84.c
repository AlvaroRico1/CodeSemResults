static size_t strip_cr_from_buf(char *start, size_t len)
{
	char *scan, *trail, *end = start + len;

	for (scan = trail = start; scan < end; trail++, scan++) {
		while (*scan == '\r')
			scan++; /* skip '\r' */

		if (trail != scan)
			*trail = *scan;
	}

	*trail = '\0';

	return (trail - start);
}


// Source: clar_libgit2.c
// Lines 488-503
