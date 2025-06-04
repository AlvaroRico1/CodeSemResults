void test_submodule_update__update_submodule(void)
{
	git_submodule *sm;
	git_submodule_update_options update_options = GIT_SUBMODULE_UPDATE_OPTIONS_INIT;
	unsigned int submodule_status = 0;
	struct update_submodule_cb_payload update_payload = { 0 };

	g_repo = setup_fixture_submodule_simple();

	update_options.checkout_opts.progress_cb = checkout_progress_cb;
	update_options.checkout_opts.progress_payload = &update_payload;

	update_options.fetch_opts.callbacks.update_tips = update_tips;
	update_options.fetch_opts.callbacks.payload = &update_payload;

	/* get the submodule */
	cl_git_pass(git_submodule_lookup(&sm, g_repo, "testrepo"));

	/* verify the initial state of the submodule */
	cl_git_pass(git_submodule_status(&submodule_status, g_repo, "testrepo", GIT_SUBMODULE_IGNORE_UNSPECIFIED));
	cl_assert_equal_i(submodule_status, GIT_SUBMODULE_STATUS_IN_HEAD |
		GIT_SUBMODULE_STATUS_IN_INDEX |
		GIT_SUBMODULE_STATUS_IN_CONFIG |
		GIT_SUBMODULE_STATUS_WD_UNINITIALIZED);

	/* initialize and update the submodule */
	cl_git_pass(git_submodule_init(sm, 0));
	cl_git_pass(git_submodule_update(sm, 0, &update_options));

	/* verify state */
	cl_git_pass(git_submodule_status(&submodule_status, g_repo, "testrepo", GIT_SUBMODULE_IGNORE_UNSPECIFIED));
	cl_assert_equal_i(submodule_status, GIT_SUBMODULE_STATUS_IN_HEAD |
		GIT_SUBMODULE_STATUS_IN_INDEX |
		GIT_SUBMODULE_STATUS_IN_CONFIG |
		GIT_SUBMODULE_STATUS_IN_WD);

	cl_assert(git_oid_streq(git_submodule_head_id(sm), "be3563ae3f795b2b4353bcce3a527ad0a4f7f644") == 0);
	cl_assert(git_oid_streq(git_submodule_wd_id(sm), "be3563ae3f795b2b4353bcce3a527ad0a4f7f644") == 0);
	cl_assert(git_oid_streq(git_submodule_index_id(sm), "be3563ae3f795b2b4353bcce3a527ad0a4f7f644") == 0);

	/* verify that the expected callbacks have been called. */
	cl_assert_equal_i(1, update_payload.checkout_progress_called);
	cl_assert_equal_i(1, update_payload.update_tips_called);

	git_submodule_free(sm);
}


// Source: update.c
// Lines 87-132
