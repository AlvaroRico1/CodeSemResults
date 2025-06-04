void test_submodule_update__update_and_init_submodule(void)
{
	git_submodule *sm;
	git_submodule_update_options update_options = GIT_SUBMODULE_UPDATE_OPTIONS_INIT;
	unsigned int submodule_status = 0;

	g_repo = setup_fixture_submodule_simple();

	/* get the submodule */
	cl_git_pass(git_submodule_lookup(&sm, g_repo, "testrepo"));

	cl_git_pass(git_submodule_status(&submodule_status, g_repo, "testrepo", GIT_SUBMODULE_IGNORE_UNSPECIFIED));
	cl_assert_equal_i(submodule_status, GIT_SUBMODULE_STATUS_IN_HEAD |
		GIT_SUBMODULE_STATUS_IN_INDEX |
		GIT_SUBMODULE_STATUS_IN_CONFIG |
		GIT_SUBMODULE_STATUS_WD_UNINITIALIZED);

	/* update (with option to initialize sub repo) */
	cl_git_pass(git_submodule_update(sm, 1, &update_options));

	/* verify expected state */
	cl_assert(git_oid_streq(git_submodule_head_id(sm), "be3563ae3f795b2b4353bcce3a527ad0a4f7f644") == 0);
	cl_assert(git_oid_streq(git_submodule_wd_id(sm), "be3563ae3f795b2b4353bcce3a527ad0a4f7f644") == 0);
	cl_assert(git_oid_streq(git_submodule_index_id(sm), "be3563ae3f795b2b4353bcce3a527ad0a4f7f644") == 0);

	git_submodule_free(sm);
}


// Source: update.c
// Lines 181-207
