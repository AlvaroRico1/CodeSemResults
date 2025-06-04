static int diff_driver_alloc(
	git_diff_driver **out, size_t *namelen_out, const char *name)
{
	git_diff_driver *driver;
	size_t driverlen = sizeof(git_diff_driver),
		namelen = strlen(name),
		alloclen;

	GIT_ERROR_CHECK_ALLOC_ADD(&alloclen, driverlen, namelen);
	GIT_ERROR_CHECK_ALLOC_ADD(&alloclen, alloclen, 1);

	driver = git__calloc(1, alloclen);
	GIT_ERROR_CHECK_ALLOC(driver);

	memcpy(driver->name, name, namelen);

	*out = driver;

	if (namelen_out)
		*namelen_out = namelen;

	return 0;
}


// Source: diff_driver.c
// Lines 161-183
