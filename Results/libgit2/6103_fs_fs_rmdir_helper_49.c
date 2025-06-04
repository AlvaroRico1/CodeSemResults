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

	cl_assert_(errno == 0, "Failed to iterate source dir");
	closedir(dir);

	cl_must_pass_(rmdir(path), "Could not remove directory");
}


// Source: fs.h
// Lines 472-493
