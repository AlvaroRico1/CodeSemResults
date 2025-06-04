void test_revwalk_basic__big_timestamp(void)
{
	git_reference *head;
	git_commit *tip;
	git_signature *sig;
	git_tree *tree;
	git_oid id;
	int error;

	revwalk_basic_setup_walk("testrepo.git");

	cl_git_pass(git_repository_head(&head, _repo));
	cl_git_pass(git_reference_peel((git_object **) &tip, head, GIT_OBJECT_COMMIT));

	/* Commit with a far-ahead timestamp, we should be able to parse it in the revwalk */
	cl_git_pass(git_signature_new(&sig, "Joe", "joe@example.com", INT64_C(2399662595), 0));
	cl_git_pass(git_commit_tree(&tree, tip));

	cl_git_pass(git_commit_create(&id, _repo, "HEAD", sig, sig, NULL, "some message", tree, 1,
		(const git_commit **)&tip));

	cl_git_pass(git_revwalk_push_head(_walk));

	while ((error = git_revwalk_next(&id, _walk)) == 0) {
		/* nothing */
	}

	cl_assert_equal_i(GIT_ITEROVER, error);

	git_tree_free(tree);
	git_commit_free(tip);
	git_reference_free(head);
	git_signature_free(sig);

}


// Source: basic.c
// Lines 518-552
