int git_branch_name(
	const char **out,
	const git_reference *ref)
{
	const char *branch_name;

	GIT_ASSERT_ARG(out);
	GIT_ASSERT_ARG(ref);

	branch_name = ref->name;

	if (git_reference_is_branch(ref)) {
		branch_name += strlen(GIT_REFS_HEADS_DIR);
	} else if (git_reference_is_remote(ref)) {
		branch_name += strlen(GIT_REFS_REMOTES_DIR);
	} else {
		git_error_set(GIT_ERROR_INVALID,
				"reference '%s' is neither a local nor a remote branch.", ref->name);
		return -1;
	}
	*out = branch_name;
	return 0;
}


// Source: branch.c
// Lines 366-388
