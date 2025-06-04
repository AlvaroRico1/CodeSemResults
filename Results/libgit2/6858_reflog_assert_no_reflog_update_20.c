static void assert_no_reflog_update(void)
{
	size_t nlogs, nlogs_after;
	size_t nlogs_master, nlogs_master_after;
	git_reference *ref;
	git_reflog *log;
	git_oid id;

	cl_git_pass(git_reflog_read(&log, g_repo, "HEAD"));
	nlogs = git_reflog_entrycount(log);
	git_reflog_free(log);

	cl_git_pass(git_reflog_read(&log, g_repo, "refs/heads/master"));
	nlogs_master = git_reflog_entrycount(log);
	git_reflog_free(log);

	/* Move it back */
	git_oid_fromstr(&id, "be3563ae3f795b2b4353bcce3a527ad0a4f7f644");
	cl_git_pass(git_reference_create(&ref, g_repo, "refs/heads/master", &id, 1, NULL));
	git_reference_free(ref);

	cl_git_pass(git_reflog_read(&log, g_repo, "HEAD"));
	nlogs_after = git_reflog_entrycount(log);
	git_reflog_free(log);

	cl_assert_equal_i(nlogs_after, nlogs);

	cl_git_pass(git_reflog_read(&log, g_repo, "refs/heads/master"));
	nlogs_master_after = git_reflog_entrycount(log);
	git_reflog_free(log);

	cl_assert_equal_i(nlogs_after, nlogs);
	cl_assert_equal_i(nlogs_master_after, nlogs_master);

}


// Source: reflog.c
// Lines 376-410
