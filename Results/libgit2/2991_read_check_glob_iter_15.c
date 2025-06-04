static void check_glob_iter(git_config *cfg, const char *regexp, int expected)
{
	git_config_iterator *iter;
	git_config_entry *entry;
	int count, error;

	cl_git_pass(git_config_iterator_glob_new(&iter, cfg, regexp));

	count = 0;
	while ((error = git_config_next(&entry, iter)) == 0)
		count++;

	cl_assert_equal_i(GIT_ITEROVER, error);
	cl_assert_equal_i(expected, count);
	git_config_iterator_free(iter);
}


// Source: read.c
// Lines 425-440
