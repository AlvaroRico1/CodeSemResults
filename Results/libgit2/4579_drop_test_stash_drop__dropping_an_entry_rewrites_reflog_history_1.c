void test_stash_drop__dropping_an_entry_rewrites_reflog_history(void)
{
	git_reference *stash;
	git_reflog *reflog;
	const git_reflog_entry *entry;
	git_oid oid;
	size_t count;

	push_three_states();

	cl_git_pass(git_reference_lookup(&stash, repo, GIT_REFS_STASH_FILE));

	cl_git_pass(git_reflog_read(&reflog, repo, GIT_REFS_STASH_FILE));
	entry = git_reflog_entry_byindex(reflog, 1);

	git_oid_cpy(&oid, git_reflog_entry_id_old(entry));
	count = git_reflog_entrycount(reflog);

	git_reflog_free(reflog);

	cl_git_pass(git_stash_drop(repo, 1));

	cl_git_pass(git_reflog_read(&reflog, repo, GIT_REFS_STASH_FILE));
	entry = git_reflog_entry_byindex(reflog, 0);

	cl_assert_equal_oid(&oid, git_reflog_entry_id_old(entry));
	cl_assert_equal_sz(count - 1, git_reflog_entrycount(reflog));

	git_reflog_free(reflog);

	git_reference_free(stash);
}


// Source: drop.c
// Lines 93-124
