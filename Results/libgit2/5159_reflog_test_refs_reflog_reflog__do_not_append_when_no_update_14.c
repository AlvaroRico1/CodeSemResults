void test_refs_reflog_reflog__do_not_append_when_no_update(void)
{
	size_t nlogs, nlogs_after;
	git_reference *ref, *ref2;
	git_reflog *log;

	cl_git_pass(git_reflog_read(&log, g_repo, "HEAD"));
	nlogs = git_reflog_entrycount(log);
	git_reflog_free(log);

	cl_git_pass(git_reference_lookup(&ref, g_repo, "refs/heads/master"));
	cl_git_pass(git_reference_create(&ref2, g_repo, "refs/heads/master",
					 git_reference_target(ref), 1, NULL));

	git_reference_free(ref);
	git_reference_free(ref2);

	cl_git_pass(git_reflog_read(&log, g_repo, "HEAD"));
	nlogs_after = git_reflog_entrycount(log);
	git_reflog_free(log);

	cl_assert_equal_i(nlogs_after, nlogs);
}


// Source: reflog.c
// Lines 352-374
