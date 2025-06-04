int git_blame_file(
		git_blame **out,
		git_repository *repo,
		const char *path,
		git_blame_options *options)
{
	int error = -1;
	git_blame_options normOptions = GIT_BLAME_OPTIONS_INIT;
	git_blame *blame = NULL;

	GIT_ASSERT_ARG(out);
	GIT_ASSERT_ARG(repo);
	GIT_ASSERT_ARG(path);

	if ((error = normalize_options(&normOptions, options, repo)) < 0)
		goto on_error;

	blame = git_blame__alloc(repo, normOptions, path);
	GIT_ERROR_CHECK_ALLOC(blame);

	if ((error = load_blob(blame)) < 0)
		goto on_error;

	if ((error = blame_internal(blame)) < 0)
		goto on_error;

	*out = blame;
	return 0;

on_error:
	git_blame_free(blame);
	return error;
}


// Source: blame.c
// Lines 387-419
