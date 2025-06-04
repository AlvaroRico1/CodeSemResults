static int futils__rmdir_recurs_foreach(void *opaque, git_str *path)
{
	int error = 0;
	futils__rmdir_data *data = opaque;
	struct stat st;

	if (data->depth > FUTILS_MAX_DEPTH)
		error = futils__error_cannot_rmdir(
			path->ptr, "directory nesting too deep");

	else if ((error = p_lstat_posixly(path->ptr, &st)) < 0) {
		if (errno == ENOENT)
			error = 0;
		else if (errno == ENOTDIR) {
			/* asked to remove a/b/c/d/e and a/b is a normal file */
			if ((data->flags & GIT_RMDIR_REMOVE_BLOCKERS) != 0)
				error = futils__rm_first_parent(path, data->base);
			else
				futils__error_cannot_rmdir(
					path->ptr, "parent is not directory");
		}
		else
			error = git_fs_path_set_error(errno, path->ptr, "rmdir");
	}

	else if (S_ISDIR(st.st_mode)) {
		data->depth++;

		error = git_fs_path_direach(path, 0, futils__rmdir_recurs_foreach, data);

		data->depth--;

		if (error < 0)
			return error;

		if (data->depth == 0 && (data->flags & GIT_RMDIR_SKIP_ROOT) != 0)
			return error;

		if ((error = p_rmdir(path->ptr)) < 0) {
			if ((data->flags & GIT_RMDIR_SKIP_NONEMPTY) != 0 &&
				(errno == ENOTEMPTY || errno == EEXIST || errno == EBUSY))
				error = 0;
			else
				error = git_fs_path_set_error(errno, path->ptr, "rmdir");
		}
	}

	else if ((data->flags & GIT_RMDIR_REMOVE_FILES) != 0) {
		if (p_unlink(path->ptr) < 0)
			error = git_fs_path_set_error(errno, path->ptr, "remove");
	}

	else if ((data->flags & GIT_RMDIR_SKIP_NONEMPTY) == 0)
		error = futils__error_cannot_rmdir(path->ptr, "still present");

	return error;
}


// Source: futils.c
// Lines 770-826
