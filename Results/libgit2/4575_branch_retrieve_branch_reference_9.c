static int retrieve_branch_reference(
	git_reference **branch_reference_out,
	git_repository *repo,
	const char *branch_name,
	bool is_remote)
{
	git_reference *branch = NULL;
	int error = 0;
	char *prefix;
	git_str ref_name = GIT_STR_INIT;

	prefix = is_remote ? GIT_REFS_REMOTES_DIR : GIT_REFS_HEADS_DIR;

	if ((error = git_str_joinpath(&ref_name, prefix, branch_name)) < 0)
		/* OOM */;
	else if ((error = git_reference_lookup(&branch, repo, ref_name.ptr)) < 0)
		git_error_set(
			GIT_ERROR_REFERENCE, "cannot locate %s branch '%s'",
			is_remote ? "remote-tracking" : "local", branch_name);

	*branch_reference_out = branch; /* will be NULL on error */

	git_str_dispose(&ref_name);
	return error;
}


// Source: branch.c
// Lines 22-46
