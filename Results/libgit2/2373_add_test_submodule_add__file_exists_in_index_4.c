void test_submodule_add__file_exists_in_index(void)
{
	git_index *index;
	git_submodule *sm;
	git_str name = GIT_STR_INIT;

	g_repo = cl_git_sandbox_init("testrepo");

	cl_git_pass(git_repository_index__weakptr(&index, g_repo));

	test_add_entry(index, valid_blob_id, "subdirectory", GIT_FILEMODE_BLOB);

	cl_git_fail_with(git_submodule_add_setup(&sm, g_repo, "./", "subdirectory", 1), GIT_EEXISTS);

	git_submodule_free(sm);
	git_str_dispose(&name);
}


// Source: add.c
// Lines 170-186
