fs_copydir_helper(const char *source, const char *dest, int dest_mode)
{
	DIR *source_dir;
	struct dirent *d;

	mkdir(dest, dest_mode);

	cl_assert_(source_dir = opendir(source), "Could not open source dir");
	while ((d = (errno = 0, readdir(source_dir))) != NULL) {
		char *child;

		if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
			continue;

		child = joinpath(source, d->d_name, -1);
		fs_copy(child, dest);
		free(child);
	}

	cl_assert_(errno == 0, "Failed to iterate source dir");

	closedir(source_dir);
}


// Source: fs.h
// Lines 367-389
