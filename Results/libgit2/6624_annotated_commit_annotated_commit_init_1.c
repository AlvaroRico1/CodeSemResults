static int annotated_commit_init(
	git_annotated_commit **out,
	git_commit *commit,
	const char *description)
{
	git_annotated_commit *annotated_commit;
	int error = 0;

	GIT_ASSERT_ARG(out);
	GIT_ASSERT_ARG(commit);

	*out = NULL;

	annotated_commit = git__calloc(1, sizeof(git_annotated_commit));
	GIT_ERROR_CHECK_ALLOC(annotated_commit);

	annotated_commit->type = GIT_ANNOTATED_COMMIT_REAL;

	if ((error = git_commit_dup(&annotated_commit->commit, commit)) < 0)
		goto done;

	git_oid_fmt(annotated_commit->id_str, git_commit_id(commit));
	annotated_commit->id_str[GIT_OID_HEXSZ] = '\0';

	if (!description)
		description = annotated_commit->id_str;

	annotated_commit->description = git__strdup(description);
	GIT_ERROR_CHECK_ALLOC(annotated_commit->description);

done:
	if (!error)
		*out = annotated_commit;

	return error;
}


// Source: annotated_commit.c
// Lines 21-56
