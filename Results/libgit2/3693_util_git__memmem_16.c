const void * git__memmem(const void *haystack, size_t haystacklen,
			 const void *needle, size_t needlelen)
{
	const char *h, *n;
	size_t j, k, l;

	if (needlelen > haystacklen || !haystacklen || !needlelen)
		return NULL;

	h = (const char *) haystack,
	n = (const char *) needle;

	if (needlelen == 1)
		return memchr(haystack, *n, haystacklen);

	if (n[0] == n[1]) {
		k = 2;
		l = 1;
	} else {
		k = 1;
		l = 2;
	}

	j = 0;
	while (j <= haystacklen - needlelen) {
		if (n[1] != h[j + 1]) {
			j += k;
		} else {
			if (memcmp(n + 2, h + j + 2, needlelen - 2) == 0 &&
			    n[0] == h[j])
				return h + j;
			j += l;
		}
	}

	return NULL;
}


// Source: util.c
// Lines 333-369
