void test_revert_workdir__nonmerge_fails_mainline_specified(void)
{
	git_reference *head;
	git_commit *commit;
	git_revert_options opts = GIT_REVERT_OPTIONS_INIT;

	cl_git_pass(git_repository_head(&head, repo));
	cl_git_pass(git_reference_peel((git_object **)&commit, head, GIT_OBJECT_COMMIT));

	opts.mainline = 1;
	cl_must_fail(git_revert(repo, commit, &opts));
	cl_assert(!git_fs_path_exists(TEST_REPO_PATH "/.git/MERGE_MSG"));
	cl_assert(!git_fs_path_exists(TEST_REPO_PATH "/.git/REVERT_HEAD"));

	git_reference_free(head);
	git_commit_free(commit);
}


// Source: workdir.c
// Lines 490-506
