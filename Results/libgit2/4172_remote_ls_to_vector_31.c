static int ls_to_vector(git_vector *out, git_remote *remote)
{
	git_remote_head **heads;
	size_t heads_len, i;

	if (git_remote_ls((const git_remote_head ***)&heads, &heads_len, remote) < 0)
		return -1;

	if (git_vector_init(out, heads_len, remote_head_cmp) < 0)
		return -1;

	for (i = 0; i < heads_len; i++) {
		if (git_vector_insert(out, heads[i]) < 0)
			return -1;
	}

	return 0;
}


// Source: remote.c
// Lines 1209-1226
