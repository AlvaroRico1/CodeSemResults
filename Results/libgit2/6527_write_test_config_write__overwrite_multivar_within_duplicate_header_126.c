void test_config_write__overwrite_multivar_within_duplicate_header(void)
{
	const char *file_name  = "config-duplicate-header";
	const char *entry_name = "remote.origin.url";
	git_config *cfg;
	git_config_entry *entry;
	int n = 0;

	/* This config can occur after removing and re-adding the origin remote */
	const char *file_content =
		"[remote \"origin\"]\n"		\
		"	url = \"bar\"\n"		\
		"[branch \"master\"]\n"		\
		"	remote = \"origin\"\n"	\
		"[remote \"origin\"]\n"		\
		"	url = \"foo\"\n";

	/* Write the test config and make sure the expected entry exists */
	cl_git_mkfile(file_name, file_content);
	cl_git_pass(git_config_open_ondisk(&cfg, file_name));
	cl_git_pass(git_config_get_entry(&entry, cfg, entry_name));

	/* Update that entry */
	cl_git_pass(git_config_set_multivar(cfg, entry_name, ".*", "newurl"));
	git_config_entry_free(entry);
	git_config_free(cfg);

	/* Reopen the file and make sure the entry was updated */
	cl_git_pass(git_config_open_ondisk(&cfg, file_name));
	cl_git_pass(git_config_get_multivar_foreach(cfg, entry_name, NULL, multivar_cb, &n));
	cl_assert_equal_i(2, n);

	/* Cleanup */
	git_config_free(cfg);
}


// Source: write.c
// Lines 247-281
