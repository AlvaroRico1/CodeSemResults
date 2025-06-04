void test_config_stress__complex(void)
{
	git_config *config;
	const char *path = "./config-immediate-multiline";

	cl_git_mkfile(path, "[imm]\n multi = \"\\\nfoo\"");
	cl_git_pass(git_config_open_ondisk(&config, path));
	assert_config_value(config, "imm.multi", "foo");

	git_config_free(config);
}


// Source: stress.c
// Lines 104-114
