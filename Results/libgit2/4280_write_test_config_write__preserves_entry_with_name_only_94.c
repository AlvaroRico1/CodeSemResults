void test_config_write__preserves_entry_with_name_only(void)
{
	const char *file_name  = "config-empty-value";
	git_config *cfg;
	git_str newfile = GIT_STR_INIT;

	/* Write the test config and make sure the expected entry exists */
	cl_git_mkfile(file_name, "[section \"foo\"]\n\tname\n");
	cl_git_pass(git_config_open_ondisk(&cfg, file_name));
	cl_git_pass(git_config_set_string(cfg, "newsection.newname", "new_value"));
	cl_git_pass(git_config_set_string(cfg, "section.foo.other", "otherval"));

	cl_git_pass(git_futils_readbuffer(&newfile, file_name));
	cl_assert_equal_s("[section \"foo\"]\n\tname\n\tother = otherval\n[newsection]\n\tnewname = new_value\n", newfile.ptr);

	git_str_dispose(&newfile);
	git_config_free(cfg);
}


// Source: write.c
// Lines 588-605
