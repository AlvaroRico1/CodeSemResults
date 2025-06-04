void test_core_env__0(void)
{
	git_str path = GIT_STR_INIT, found = GIT_STR_INIT;
	char testfile[16], tidx = '0';
	char **val;
	const char *testname = "testfile";
	size_t testlen = strlen(testname);

	strncpy(testfile, testname, sizeof(testfile));
	cl_assert_equal_s(testname, testfile);

	for (val = home_values; *val != NULL; val++) {

		/* if we can't make the directory, let's just assume
		 * we are on a filesystem that doesn't support the
		 * characters in question and skip this test...
		 */
		if (p_mkdir(*val, 0777) != 0) {
			*val = ""; /* mark as not created */
			continue;
		}

		cl_git_pass(git_fs_path_prettify(&path, *val, NULL));

		/* vary testfile name in each directory so accidentally leaving
		 * an environment variable set from a previous iteration won't
		 * accidentally make this test pass...
		 */
		testfile[testlen] = tidx++;
		cl_git_pass(git_str_joinpath(&path, path.ptr, testfile));
		cl_git_mkfile(path.ptr, "find me");
		git_str_rtruncate_at_char(&path, '/');

		cl_assert_equal_i(
			GIT_ENOTFOUND, git_sysdir_find_global_file(&found, testfile));

		setenv_and_check("HOME", path.ptr);
		set_global_search_path_from_env();

		cl_git_pass(git_sysdir_find_global_file(&found, testfile));

		cl_setenv("HOME", env_save[0]);
		set_global_search_path_from_env();

		cl_assert_equal_i(
			GIT_ENOTFOUND, git_sysdir_find_global_file(&found, testfile));

#ifdef GIT_WIN32
		setenv_and_check("HOMEDRIVE", NULL);
		setenv_and_check("HOMEPATH", NULL);
		setenv_and_check("USERPROFILE", path.ptr);
		set_global_search_path_from_env();

		cl_git_pass(git_sysdir_find_global_file(&found, testfile));

		{
			int root = git_fs_path_root(path.ptr);
			char old;

			if (root >= 0) {
				setenv_and_check("USERPROFILE", NULL);
				set_global_search_path_from_env();

				cl_assert_equal_i(
					GIT_ENOTFOUND, git_sysdir_find_global_file(&found, testfile));

				old = path.ptr[root];
				path.ptr[root] = '\0';
				setenv_and_check("HOMEDRIVE", path.ptr);
				path.ptr[root] = old;
				setenv_and_check("HOMEPATH", &path.ptr[root]);
				set_global_search_path_from_env();

				cl_git_pass(git_sysdir_find_global_file(&found, testfile));
			}
		}
#endif

		(void)p_rmdir(*val);
	}

	git_str_dispose(&path);
	git_str_dispose(&found);
}


// Source: env.c
// Lines 83-166
