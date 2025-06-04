void test_commit_parse__entire_commit(void)
{
	const int failing_commit_count = ARRAY_SIZE(failing_commit_cases);
	const int passing_commit_count = ARRAY_SIZE(passing_commit_cases);
	int i;
	git_commit *commit;

	for (i = 0; i < failing_commit_count; ++i) {
		cl_git_fail(parse_commit(&commit, failing_commit_cases[i]));
		git_commit__free(commit);
	}

	for (i = 0; i < passing_commit_count; ++i) {
		cl_git_pass(parse_commit(&commit, passing_commit_cases[i]));

		if (!i)
			cl_assert_equal_s("", git_commit_message(commit));
		else
			cl_assert(git__prefixcmp(
				git_commit_message(commit), "a simple commit which works") == 0);

		git_commit__free(commit);
	}
}


// Source: parse.c
// Lines 294-317
