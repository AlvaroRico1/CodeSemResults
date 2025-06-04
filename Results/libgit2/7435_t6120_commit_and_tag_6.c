static void commit_and_tag(
	git_time_t *time,
	const char *commit_msg,
	const char *tag_name)
{
	git_index *index;
	git_oid commit_id;
	git_reference *ref;
	
	cl_git_pass(git_repository_index__weakptr(&index, repo));

	cl_git_append2file("describe/file", "\n");
	
	cl_git_pass(git_index_add_bypath(index, "file"));
	cl_git_pass(git_index_write(index));

	*time += 10;
	cl_repo_commit_from_index(&commit_id, repo, NULL, *time, commit_msg);

	if (tag_name == NULL)
		return;

	cl_git_pass(git_reference_create(&ref, repo, tag_name, &commit_id, 0, NULL));
	git_reference_free(ref);
}


// Source: t6120.c
// Lines 94-118
