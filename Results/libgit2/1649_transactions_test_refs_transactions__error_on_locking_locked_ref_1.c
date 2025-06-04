void test_refs_transactions__error_on_locking_locked_ref(void)
{
	git_oid id;
	git_transaction *g_tx_with_lock;
	git_repository *g_repo_with_locking_tx;
	const char *g_repo_path = git_repository_path(g_repo);
	
	/* prepare a separate transaction in another instance of testrepo and lock master */
	cl_git_pass(git_repository_open(&g_repo_with_locking_tx, g_repo_path));
	cl_git_pass(git_transaction_new(&g_tx_with_lock, g_repo_with_locking_tx));
	cl_git_pass(git_transaction_lock_ref(g_tx_with_lock, "refs/heads/master"));

	/* lock reference for set_target */
	cl_git_pass(git_oid_fromstr(&id, "a65fedf39aefe402d3bb6e24df4d4f5fe4547750"));
	cl_git_fail_with(GIT_ELOCKED, git_transaction_lock_ref(g_tx, "refs/heads/master"));
	cl_git_fail_with(GIT_ENOTFOUND, git_transaction_set_target(g_tx, "refs/heads/master", &id, NULL, NULL));

	git_transaction_free(g_tx_with_lock);
	git_repository_free(g_repo_with_locking_tx);
}


// Source: transactions.c
// Lines 112-131
