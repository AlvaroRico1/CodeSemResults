fs_rmdir_helper(const char *path)
{
	DIR *dir;
	struct dirent *d;

	cl_assert_(dir = opendir(path), "Could not open dir");
	while ((d = (errno = 0, readdir(dir))) != NULL) {
		char *child;

		if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
			continue;

		child = joinpath(path, d->d_name, -1);
		fs_rm(child);
		free(child);
	}


// Source: fs.h
// Lines 472-487
