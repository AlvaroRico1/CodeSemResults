int git_libgit2_opts(int key, ...)
{
	int error = 0;
	va_list ap;

	va_start(ap, key);

	switch (key) {
	case GIT_OPT_SET_MWINDOW_SIZE:
		git_mwindow__window_size = va_arg(ap, size_t);
		break;

	case GIT_OPT_GET_MWINDOW_SIZE:
		*(va_arg(ap, size_t *)) = git_mwindow__window_size;
		break;

	case GIT_OPT_SET_MWINDOW_MAPPED_LIMIT:
		git_mwindow__mapped_limit = va_arg(ap, size_t);
		break;

	case GIT_OPT_GET_MWINDOW_MAPPED_LIMIT:
		*(va_arg(ap, size_t *)) = git_mwindow__mapped_limit;
		break;

	case GIT_OPT_SET_MWINDOW_FILE_LIMIT:
		git_mwindow__file_limit = va_arg(ap, size_t);
		break;

	case GIT_OPT_GET_MWINDOW_FILE_LIMIT:
		*(va_arg(ap, size_t *)) = git_mwindow__file_limit;
		break;

	case GIT_OPT_GET_SEARCH_PATH:
		{
			int sysdir = va_arg(ap, int);
			git_buf *out = va_arg(ap, git_buf *);
			git_str str = GIT_STR_INIT;
			const git_str *tmp;
			int level;

			if ((error = git_buf_tostr(&str, out)) < 0 ||
			    (error = config_level_to_sysdir(&level, sysdir)) < 0 ||
			    (error = git_sysdir_get(&tmp, level)) < 0 ||
			    (error = git_str_put(&str, tmp->ptr, tmp->size)) < 0)
				break;

			error = git_buf_fromstr(out, &str);
		}
		break;

	case GIT_OPT_SET_SEARCH_PATH:
		{
			int level;

			if ((error = config_level_to_sysdir(&level, va_arg(ap, int))) >= 0)
				error = git_sysdir_set(level, va_arg(ap, const char *));
		}


// Source: libgit2.c
// Lines 167-223
