int git_str_lf_to_crlf(git_str *tgt, const git_str *src)
{
	const char *start = src->ptr;
	const char *end = start + src->size;
	const char *scan = start;
	const char *next = memchr(scan, '\n', src->size);
	size_t alloclen;

	GIT_ASSERT(tgt != src);

	if (!next)
		return git_str_set(tgt, src->ptr, src->size);

	/* attempt to reduce reallocs while in the loop */
	GIT_ERROR_CHECK_ALLOC_ADD(&alloclen, src->size, src->size >> 4);
	GIT_ERROR_CHECK_ALLOC_ADD(&alloclen, alloclen, 1);
	if (git_str_grow(tgt, alloclen) < 0)
		return -1;
	tgt->size = 0;

	for (; next; scan = next + 1, next = memchr(scan, '\n', end - scan)) {
		size_t copylen = next - scan;

		/* if we find mixed line endings, carry on */
		if (copylen && next[-1] == '\r')
			copylen--;

		GIT_ERROR_CHECK_ALLOC_ADD(&alloclen, copylen, 3);
		if (git_str_grow_by(tgt, alloclen) < 0)
			return -1;

		if (copylen) {
			memcpy(tgt->ptr + tgt->size, scan, copylen);
			tgt->size += copylen;
		}

		tgt->ptr[tgt->size++] = '\r';
		tgt->ptr[tgt->size++] = '\n';
	}

	tgt->ptr[tgt->size] = '\0';
	return git_str_put(tgt, scan, end - scan);
}


// Source: str.c
// Lines 1166-1208
