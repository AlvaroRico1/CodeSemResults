static void check_get_multivar_foreach(
	git_config *cfg, int expected, int expected_patterned)
{
	int n = 0;

	if (expected > 0) {
		cl_git_pass(git_config_get_multivar_foreach(cfg, _name, NULL, cb, &n));
		cl_assert_equal_i(expected, n);
	} else {
		cl_assert_equal_i(GIT_ENOTFOUND,
			git_config_get_multivar_foreach(cfg, _name, NULL, cb, &n));
	}

	n = 0;

	if (expected_patterned > 0) {
		cl_git_pass(git_config_get_multivar_foreach(cfg, _name, "example", cb, &n));
		cl_assert_equal_i(expected_patterned, n);
	} else {
		cl_assert_equal_i(GIT_ENOTFOUND,
			git_config_get_multivar_foreach(cfg, _name, "example", cb, &n));
	}
}


// Source: multivar.c
// Lines 49-71
