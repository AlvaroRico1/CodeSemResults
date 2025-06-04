void test_attr_repo__get_one_start_deep(void)
{
	int i;

	for (i = (int)ARRAY_SIZE(get_one_test_cases) - 1; i >= 0; --i) {
		struct attr_expected *scan = &get_one_test_cases[i];
		const char *value;

		cl_git_pass(git_attr_get(&value, g_repo, 0, scan->path, scan->attr));
		attr_check_expected(
			scan->expected, scan->expected_str, scan->attr, value);
	}


// Source: repo.c
// Lines 81-92
