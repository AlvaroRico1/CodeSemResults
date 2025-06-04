void test_refs_reflog_reflog__append_to_HEAD_when_changing_current_branch(void)
{
	size_t nlogs, nlogs_after;
	git_reference *ref;
	git_reflog *log;
	git_oid id;

	cl_git_pass(git_reflog_read(&log, g_repo, "HEAD"));
	nlogs = git_reflog_entrycount(log);
	git_reflog_free(log);

	/* Move it back */
	git_oid_fromstr(&id, "be3563ae3f795b2b4353bcce3a527ad0a4f7f644");
	cl_git_pass(git_reference_create(&ref, g_repo, "refs/heads/master", &id, 1, NULL));
	git_reference_free(ref);

	cl_git_pass(git_reflog_read(&log, g_repo, "HEAD"));
	nlogs_after = git_reflog_entrycount(log);
	git_reflog_free(log);

	cl_assert_equal_i(nlogs_after, nlogs + 1);
}


// Source: reflog.c
// Lines 329-350
