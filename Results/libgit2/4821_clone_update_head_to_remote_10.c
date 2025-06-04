static int update_head_to_remote(
		git_repository *repo,
		git_remote *remote,
		const char *reflog_message)
{
	int error = 0;
	size_t refs_len;
	const git_remote_head *remote_head, **refs;
	const git_oid *remote_head_id;
	git_str branch = GIT_STR_INIT;

	if ((error = git_remote_ls(&refs, &refs_len, remote)) < 0)
		return error;

	/* We cloned an empty repository or one with an unborn HEAD */
	if (refs_len == 0 || strcmp(refs[0]->name, GIT_HEAD_FILE))
		return update_head_to_default(repo);

	/* We know we have HEAD, let's see where it points */
	remote_head = refs[0];
	GIT_ASSERT(remote_head);

	remote_head_id = &remote_head->oid;

	error = git_remote__default_branch(&branch, remote);
	if (error == GIT_ENOTFOUND) {
		error = git_repository_set_head_detached(
			repo, remote_head_id);
		goto cleanup;
	}

	if ((error = update_remote_head(repo, remote, &branch, reflog_message)) < 0)
		goto cleanup;

	error = update_head_to_new_branch(
		repo,
		remote_head_id,
		git_str_cstr(&branch),
		reflog_message);

cleanup:
	git_str_dispose(&branch);

	return error;
}


// Source: clone.c
// Lines 214-258
