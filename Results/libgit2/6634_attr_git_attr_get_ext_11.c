int git_attr_get_ext(
	const char **value,
	git_repository *repo,
	git_attr_options *opts,
	const char *pathname,
	const char *name)
{
	int error;
	git_attr_path path;
	git_vector files = GIT_VECTOR_INIT;
	size_t i, j;
	git_attr_file *file;
	git_attr_name attr;
	git_attr_rule *rule;
	git_dir_flag dir_flag = GIT_DIR_FLAG_UNKNOWN;

	GIT_ASSERT_ARG(value);
	GIT_ASSERT_ARG(repo);
	GIT_ASSERT_ARG(name);
	GIT_ERROR_CHECK_VERSION(opts, GIT_ATTR_OPTIONS_VERSION, "git_attr_options");

	*value = NULL;

	if (git_repository_is_bare(repo))
		dir_flag = GIT_DIR_FLAG_FALSE;

	if (git_attr_path__init(&path, pathname, git_repository_workdir(repo), dir_flag) < 0)
		return -1;

	if ((error = collect_attr_files(repo, NULL, opts, pathname, &files)) < 0)
		goto cleanup;

	memset(&attr, 0, sizeof(attr));
	attr.name = name;
	attr.name_hash = git_attr_file__name_hash(name);

	git_vector_foreach(&files, i, file) {

		git_attr_file__foreach_matching_rule(file, &path, j, rule) {
			size_t pos;

			if (!git_vector_bsearch(&pos, &rule->assigns, &attr)) {
				*value = ((git_attr_assignment *)git_vector_get(
							  &rule->assigns, pos))->value;
				goto cleanup;
			}
		}
	}

cleanup:
	release_attr_files(&files);
	git_attr_path__free(&path);

	return error;
}


// Source: attr.c
// Lines 45-99
