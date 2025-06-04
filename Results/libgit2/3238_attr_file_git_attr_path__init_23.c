int git_attr_path__init(
	git_attr_path *info,
	const char *path,
	const char *base,
	git_dir_flag dir_flag)
{
	ssize_t root;

	/* build full path as best we can */
	git_str_init(&info->full, 0);

	if (git_fs_path_join_unrooted(&info->full, path, base, &root) < 0)
		return -1;

	info->path = info->full.ptr + root;

	/* remove trailing slashes */
	while (info->full.size > 0) {
		if (info->full.ptr[info->full.size - 1] != '/')
			break;
		info->full.size--;
	}
	info->full.ptr[info->full.size] = '\0';

	/* skip leading slashes in path */
	while (*info->path == '/')
		info->path++;

	/* find trailing basename component */
	info->basename = strrchr(info->path, '/');
	if (info->basename)
		info->basename++;
	if (!info->basename || !*info->basename)
		info->basename = info->path;

	switch (dir_flag)
	{
	case GIT_DIR_FLAG_FALSE:
		info->is_dir = 0;
		break;

	case GIT_DIR_FLAG_TRUE:
		info->is_dir = 1;
		break;

	case GIT_DIR_FLAG_UNKNOWN:
	default:
		info->is_dir = (int)git_fs_path_isdir(info->full.ptr);
		break;
	}

	return 0;
}


// Source: attr_file.c
// Lines 552-604
