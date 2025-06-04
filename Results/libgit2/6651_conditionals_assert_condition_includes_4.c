static void assert_condition_includes(const char *keyword, const char *path, bool expected)
{
	git_buf value = GIT_BUF_INIT;
	git_str buf = GIT_STR_INIT;
	git_config *cfg;

	cl_git_pass(git_str_printf(&buf, "[includeIf \"%s:%s\"]\n", keyword, path));
	cl_git_pass(git_str_puts(&buf, "path = other\n"));

	cl_git_mkfile("empty_standard_repo/.git/config", buf.ptr);
	cl_git_mkfile("empty_standard_repo/.git/other", "[foo]\nbar=baz\n");
	_repo = cl_git_sandbox_reopen();

	git_str_dispose(&buf);

	cl_git_pass(git_repository_config(&cfg, _repo));

	if (expected) {
		cl_git_pass(git_config_get_string_buf(&value, cfg, "foo.bar"));
		cl_assert_equal_s("baz", value.ptr);
	} else {
		cl_git_fail_with(GIT_ENOTFOUND,
				 git_config_get_string_buf(&value, cfg, "foo.bar"));
	}

	git_str_dispose(&buf);
	git_buf_dispose(&value);
	git_config_free(cfg);
}


// Source: conditionals.c
// Lines 23-51
