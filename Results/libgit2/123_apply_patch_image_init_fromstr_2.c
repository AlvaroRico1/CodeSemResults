static int patch_image_init_fromstr(
	patch_image *out, const char *in, size_t in_len)
{
	git_diff_line *line;
	const char *start, *end;

	memset(out, 0x0, sizeof(patch_image));

	if (git_pool_init(&out->pool, sizeof(git_diff_line)) < 0)
		return -1;

	if (!in_len)
		return 0;

	for (start = in; start < in + in_len; start = end) {
		end = memchr(start, '\n', in_len - (start - in));

		if (end == NULL)
			end = in + in_len;

		else if (end < in + in_len)
			end++;

		line = git_pool_mallocz(&out->pool, 1);
		GIT_ERROR_CHECK_ALLOC(line);

		if (git_vector_insert(&out->lines, line) < 0)
			return -1;

		patch_line_init(line, start, (end - start), (start - in));
	}

	return 0;
}


// Source: apply.c
// Lines 57-90
