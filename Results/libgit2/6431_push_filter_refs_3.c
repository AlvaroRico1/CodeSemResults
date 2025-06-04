static int filter_refs(git_remote *remote)
{
	const git_remote_head **heads;
	size_t heads_len, i;

	git_vector_clear(&remote->refs);

	if (git_remote_ls(&heads, &heads_len, remote) < 0)
		return -1;

	for (i = 0; i < heads_len; i++) {
		if (git_vector_insert(&remote->refs, (void *)heads[i]) < 0)
			return -1;
	}

	return 0;
}


// Source: push.c
// Lines 451-467
