int git_reference__update_for_commit(
	git_repository *repo,
	git_reference *ref,
	const char *ref_name,
	const git_oid *id,
	const char *operation)
{
	git_reference *ref_new = NULL;
	git_commit *commit = NULL;
	git_str reflog_msg = GIT_STR_INIT;
	const git_signature *who;
	int error;

	if ((error = git_commit_lookup(&commit, repo, id)) < 0 ||
		(error = git_str_printf(&reflog_msg, "%s%s: %s",
			operation ? operation : "commit",
			commit_type(commit),
			git_commit_summary(commit))) < 0)
		goto done;

	who = git_commit_committer(commit);

	if (ref) {
		if ((error = ensure_is_an_updatable_direct_reference(ref)) < 0)
			return error;

		error = reference__create(&ref_new, repo, ref->name, id, NULL, 1, who,
					  git_str_cstr(&reflog_msg), &ref->target.oid, NULL);
	}
	else
		error = git_reference__update_terminal(
			repo, ref_name, id, who, git_str_cstr(&reflog_msg));

done:
	git_reference_free(ref_new);
	git_str_dispose(&reflog_msg);
	git_commit_free(commit);
	return error;
}


// Source: refs.c
// Lines 1137-1175
