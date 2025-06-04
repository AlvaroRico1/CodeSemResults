int git_repository_head_detached(git_repository *repo)
{
	git_reference *ref;
	git_odb *odb = NULL;
	int exists;

	if (git_repository_odb__weakptr(&odb, repo) < 0)
		return -1;

	if (git_reference_lookup(&ref, repo, GIT_HEAD_FILE) < 0)
		return -1;

	if (git_reference_type(ref) == GIT_REFERENCE_SYMBOLIC) {
		git_reference_free(ref);
		return 0;
	}

	exists = git_odb_exists(odb, git_reference_target(ref));

	git_reference_free(ref);
	return exists;
}


// Source: repository.c
// Lines 2345-2366
