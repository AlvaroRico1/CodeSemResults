static int merge_driver_name_for_path(
	const char **out,
	git_repository *repo,
	const char *path,
	const char *default_driver)
{
	const char *value;
	int error;

	*out = NULL;

	if ((error = git_attr_get(&value, repo, 0, path, "merge")) < 0)
		return error;

	/* set: use the built-in 3-way merge driver ("text") */
	if (GIT_ATTR_IS_TRUE(value))
		*out = merge_driver_name__text;

	/* unset: do not merge ("binary") */
	else if (GIT_ATTR_IS_FALSE(value))
		*out = merge_driver_name__binary;

	else if (GIT_ATTR_IS_UNSPECIFIED(value) && default_driver)
		*out = default_driver;

	else if (GIT_ATTR_IS_UNSPECIFIED(value))
		*out = merge_driver_name__text;

	else
		*out = value;

	return 0;
}


// Source: merge_driver.c
// Lines 365-397
