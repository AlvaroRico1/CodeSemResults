static int make_submodule_dirty(git_submodule *sm, const char *name, void *payload)
{
	git_str submodulepath = GIT_STR_INIT;
	git_str dirtypath = GIT_STR_INIT;
	git_repository *submodule_repo;

	GIT_UNUSED(name);
	GIT_UNUSED(payload);

	/* remove submodule directory in preparation for init and repo_init */
	cl_git_pass(git_str_joinpath(
		&submodulepath,
		git_repository_workdir(g_repo),
		git_submodule_path(sm)
	));
	git_futils_rmdir_r(git_str_cstr(&submodulepath), NULL, GIT_RMDIR_REMOVE_FILES);

	/* initialize submodule's repository */
	cl_git_pass(git_submodule_repo_init(&submodule_repo, sm, 0));

	/* create a file in the submodule workdir to make it dirty */
	cl_git_pass(
		git_str_joinpath(&dirtypath, git_repository_workdir(submodule_repo), "dirty"));
	force_create_file(git_str_cstr(&dirtypath));

	git_str_dispose(&dirtypath);
	git_str_dispose(&submodulepath);
	git_repository_free(submodule_repo);

	return 0;
}


// Source: typechange.c
// Lines 227-257
