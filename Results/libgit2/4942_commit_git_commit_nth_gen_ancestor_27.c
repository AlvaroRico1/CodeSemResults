int git_commit_nth_gen_ancestor(
	git_commit **ancestor,
	const git_commit *commit,
	unsigned int n)
{
	git_commit *current, *parent = NULL;
	int error;

	GIT_ASSERT_ARG(ancestor);
	GIT_ASSERT_ARG(commit);

	if (git_commit_dup(&current, (git_commit *)commit) < 0)
		return -1;

	if (n == 0) {
		*ancestor = current;
		return 0;
	}

	while (n--) {
		error = git_commit_parent(&parent, current, 0);

		git_commit_free(current);

		if (error < 0)
			return error;

		current = parent;
	}

	*ancestor = parent;
	return 0;
}


// Source: commit.c
// Lines 659-691
