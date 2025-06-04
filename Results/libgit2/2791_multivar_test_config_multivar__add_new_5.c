void test_config_multivar__add_new(void)
{
	const char *var = "a.brand.new";
	git_config *cfg;
	int n;

	cl_git_pass(git_config_open_ondisk(&cfg, "config/config11"));

	cl_git_pass(git_config_set_multivar(cfg, var, "$^", "variable"));
	n = 0;
	cl_git_pass(git_config_get_multivar_foreach(cfg, var, NULL, cb, &n));
	cl_assert_equal_i(n, 1);

	git_config_free(cfg);
}


// Source: multivar.c
// Lines 158-172
