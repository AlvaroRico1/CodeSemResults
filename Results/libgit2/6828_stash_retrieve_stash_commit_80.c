static int retrieve_stash_commit(
	git_commit **commit,
	git_repository *repo,
	size_t index)
{
	git_reference *stash = NULL;
	git_reflog *reflog = NULL;
	int error;
	size_t max;
	const git_reflog_entry *entry;

	if ((error = git_reference_lookup(&stash, repo, GIT_REFS_STASH_FILE)) < 0)
		goto cleanup;

	if ((error = git_reflog_read(&reflog, repo, GIT_REFS_STASH_FILE)) < 0)
		goto cleanup;

	max = git_reflog_entrycount(reflog);
	if (!max || index > max - 1) {
		error = GIT_ENOTFOUND;
		git_error_set(GIT_ERROR_STASH, "no stashed state at position %" PRIuZ, index);
		goto cleanup;
	}

	entry = git_reflog_entry_byindex(reflog, index);
	if ((error = git_commit_lookup(commit, repo, git_reflog_entry_id_new(entry))) < 0)
		goto cleanup;

cleanup:
	git_reference_free(stash);
	git_reflog_free(reflog);
	return error;
}


// Source: stash.c
// Lines 600-632
