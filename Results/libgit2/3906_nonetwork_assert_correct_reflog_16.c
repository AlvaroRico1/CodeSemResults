static void assert_correct_reflog(const char *name)
{
	git_reflog *log;
	const git_reflog_entry *entry;
	git_str expected_message = GIT_STR_INIT;

	git_str_printf(&expected_message,
		"clone: from %s", cl_git_fixture_url("testrepo.git"));

	cl_git_pass(git_reflog_read(&log, g_repo, name));
	cl_assert_equal_i(1, git_reflog_entrycount(log));
	entry = git_reflog_entry_byindex(log, 0);
	cl_assert_equal_s(expected_message.ptr, git_reflog_entry_message(entry));

	git_reflog_free(log);

	git_str_dispose(&expected_message);
}


// Source: nonetwork.c
// Lines 303-320
