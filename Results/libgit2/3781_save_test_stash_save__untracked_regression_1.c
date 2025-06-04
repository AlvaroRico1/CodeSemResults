void test_stash_save__untracked_regression(void)
{
	git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
	const char *paths[] = {"what", "where", "how", "why"};
	git_reference *head;
	git_commit *head_commit;
	git_str untracked_dir;

	const char* workdir = git_repository_workdir(repo);

	git_str_init(&untracked_dir, 0);
	git_str_printf(&untracked_dir, "%sz", workdir);

	cl_assert(!p_mkdir(untracked_dir.ptr, 0777));

	cl_git_pass(git_repository_head(&head, repo));

	cl_git_pass(git_reference_peel((git_object **)&head_commit, head, GIT_OBJECT_COMMIT));

	opts.checkout_strategy = GIT_CHECKOUT_FORCE;

	opts.paths.strings = (char **)paths;
	opts.paths.count = 4;

	cl_git_pass(git_checkout_tree(repo, (git_object*)head_commit, &opts));

	cl_git_pass(git_stash_save(&stash_tip_oid, repo, signature, NULL, GIT_STASH_DEFAULT));

	assert_commit_message_contains("refs/stash", "WIP on master");

	git_reference_free(head);
	git_commit_free(head_commit);
	git_str_dispose(&untracked_dir);
}


// Source: save.c
// Lines 198-231
