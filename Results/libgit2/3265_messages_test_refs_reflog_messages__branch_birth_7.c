void test_refs_reflog_messages__branch_birth(void)
{
	git_signature *sig;
	git_oid id;
	git_tree *tree;
	git_reference *ref;
	const char *msg;
	size_t nentries, nentries_after;

	nentries = reflog_entrycount(g_repo, GIT_HEAD_FILE);

	cl_git_pass(git_signature_now(&sig, "me", "foo@example.com"));

	cl_git_pass(git_repository_head(&ref, g_repo));
	cl_git_pass(git_reference_peel((git_object **) &tree, ref, GIT_OBJECT_TREE));

	cl_git_pass(git_repository_set_head(g_repo, "refs/heads/orphan"));

	nentries_after = reflog_entrycount(g_repo, GIT_HEAD_FILE);

	cl_assert_equal_i(nentries, nentries_after);

	msg = "message 2";
	cl_git_pass(git_commit_create(&id, g_repo, "HEAD", sig, sig, NULL, msg, tree, 0, NULL));

	cl_assert_equal_i(1, reflog_entrycount(g_repo, "refs/heads/orphan"));

	nentries_after = reflog_entrycount(g_repo, GIT_HEAD_FILE);

	cl_assert_equal_i(nentries + 1, nentries_after);

	git_signature_free(sig);
	git_tree_free(tree);
	git_reference_free(ref);
}


// Source: messages.c
// Lines 135-169
