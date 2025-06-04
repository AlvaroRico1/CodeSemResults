static void check_get_multivar(git_config *cfg, int expected)
{
	git_config_iterator *iter;
	git_config_entry *entry;
	int n = 0;

	cl_git_pass(git_config_multivar_iterator_new(&iter, cfg, _name, NULL));

	while (git_config_next(&entry, iter) == 0)
		n++;

	cl_assert_equal_i(expected, n);
	git_config_iterator_free(iter);

}


// Source: multivar.c
// Lines 73-87
