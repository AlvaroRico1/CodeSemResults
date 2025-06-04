void test_checkout_typechange__checkout_with_conflicts(void)
{
	int i;
	git_object *obj;
	git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
	notify_counts cts = {0};

	opts.notify_flags =
		GIT_CHECKOUT_NOTIFY_CONFLICT | GIT_CHECKOUT_NOTIFY_UNTRACKED;
	opts.notify_cb = notify_counter;
	opts.notify_payload = &cts;

	for (i = 0; g_typechange_oids[i] != NULL; ++i) {
		cl_git_pass(git_revparse_single(&obj, g_repo, g_typechange_oids[i]));

		force_create_file("typechanges/a/blocker");
		force_create_file("typechanges/b");
		force_create_file("typechanges/c/sub/sub/file");
		git_futils_rmdir_r("typechanges/d", NULL, GIT_RMDIR_REMOVE_FILES);
		p_mkdir("typechanges/d", 0777); /* intentionally empty dir */
		force_create_file("typechanges/untracked");
		cl_git_pass(git_submodule_foreach(g_repo, make_submodule_dirty, NULL));

		opts.checkout_strategy = GIT_CHECKOUT_SAFE;
		memset(&cts, 0, sizeof(cts));

		cl_git_fail(git_checkout_tree(g_repo, obj, &opts));
		cl_assert_equal_i(cts.conflicts, g_typechange_expected_conflicts[i]);
		cl_assert_equal_i(cts.untracked, g_typechange_expected_untracked[i]);
		cl_assert_equal_i(cts.dirty, 0);
		cl_assert_equal_i(cts.updates, 0);
		cl_assert_equal_i(cts.ignored, 0);

		opts.checkout_strategy =
			GIT_CHECKOUT_FORCE | GIT_CHECKOUT_REMOVE_UNTRACKED;
		memset(&cts, 0, sizeof(cts));

		cl_assert(git_fs_path_exists("typechanges/untracked"));

		cl_git_pass(git_checkout_tree(g_repo, obj, &opts));
		cl_assert_equal_i(0, cts.conflicts);

		cl_assert(!git_fs_path_exists("typechanges/untracked"));

		cl_git_pass(
			git_repository_set_head_detached(g_repo, git_object_id(obj)));

		assert_workdir_matches_tree(g_repo, git_object_id(obj), NULL, true);

		git_object_free(obj);
	}
}


// Source: typechange.c
// Lines 259-310
