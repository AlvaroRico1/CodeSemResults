static int futils__rmdir_empty_parent(void *opaque, const char *path)
{
	futils__rmdir_data *data = opaque;
	int error = 0;

	if (strlen(path) <= data->baselen)
		error = GIT_ITEROVER;

	else if (p_rmdir(path) < 0) {
		int en = errno;

		if (en == ENOENT || en == ENOTDIR) {
			/* do nothing */
		} else if ((data->flags & GIT_RMDIR_SKIP_NONEMPTY) == 0 &&
			en == EBUSY) {
			error = git_fs_path_set_error(errno, path, "rmdir");
		} else if (en == ENOTEMPTY || en == EEXIST || en == EBUSY) {
			error = GIT_ITEROVER;
		} else {
			error = git_fs_path_set_error(errno, path, "rmdir");
		}
	}

	return error;
}


// Source: futils.c
// Lines 828-852
