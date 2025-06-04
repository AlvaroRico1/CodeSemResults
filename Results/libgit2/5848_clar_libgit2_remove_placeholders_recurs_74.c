static int remove_placeholders_recurs(void *_data, git_str *path)
{
	remove_data *data = (remove_data *)_data;
	size_t pathlen;

	if (git_fs_path_isdir(path->ptr) == true)
		return git_fs_path_direach(path, 0, remove_placeholders_recurs, data);

	pathlen = path->size;

	if (pathlen < data->filename_len)
		return 0;

	/* if path ends in '/'+filename (or equals filename) */
	if (!strcmp(data->filename, path->ptr + pathlen - data->filename_len) &&
		(pathlen == data->filename_len ||
		 path->ptr[pathlen - data->filename_len - 1] == '/'))
		return p_unlink(path->ptr);

	return 0;
}


// Source: clar_libgit2.c
// Lines 357-377
