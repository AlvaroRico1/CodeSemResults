static void run_test_cases(git_attr_file *file, struct attr_expected *cases, int force_dir)
{
	git_attr_path path;
	const char *value = NULL;
	struct attr_expected *c;
	int error;

	for (c = cases; c->path != NULL; c++) {
		cl_git_pass(git_attr_path__init(&path, c->path, NULL, GIT_DIR_FLAG_UNKNOWN));

		if (force_dir)
			path.is_dir = 1;

		error = git_attr_file__lookup_one(file,&path,c->attr,&value);
		cl_git_pass(error);

		attr_check_expected(c->expected, c->expected_str, c->attr, value);

		git_attr_path__free(&path);
	}
}


// Source: lookup.c
// Lines 31-51
