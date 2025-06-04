int git_str_splice(
	git_str *buf,
	size_t where,
	size_t nb_to_remove,
	const char *data,
	size_t nb_to_insert)
{
	char *splice_loc;
	size_t new_size, alloc_size;

	GIT_ASSERT(buf);
	GIT_ASSERT(where <= buf->size);
	GIT_ASSERT(nb_to_remove <= buf->size - where);

	splice_loc = buf->ptr + where;

	/* Ported from git.git
	 * https://github.com/git/git/blob/16eed7c/strbuf.c#L159-176
	 */
	GIT_ERROR_CHECK_ALLOC_ADD(&new_size, (buf->size - nb_to_remove), nb_to_insert);
	GIT_ERROR_CHECK_ALLOC_ADD(&alloc_size, new_size, 1);
	ENSURE_SIZE(buf, alloc_size);

	memmove(splice_loc + nb_to_insert,
		splice_loc + nb_to_remove,
		buf->size - where - nb_to_remove);

	memcpy(splice_loc, data, nb_to_insert);

	buf->size = new_size;
	buf->ptr[buf->size] = '\0';
	return 0;
}


// Source: str.c
// Lines 889-921
