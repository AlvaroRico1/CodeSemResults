static char *get_filename(const char *in)
{
	char *search_dirname, *search_filename, *filename = NULL;
	git_str out = GIT_STR_INIT;
	DIR *dir;
	struct dirent *de;

	cl_assert(search_dirname = git_fs_path_dirname(in));
	cl_assert(search_filename = git_fs_path_basename(in));

	cl_assert(dir = opendir(search_dirname));

	while ((de = readdir(dir))) {
		if (strcasecmp(de->d_name, search_filename) == 0) {
			git_str_join(&out, '/', search_dirname, de->d_name);
			filename = git_str_detach(&out);
			break;
		}
	}

	closedir(dir);

	git__free(search_dirname);
	git__free(search_filename);
	git_str_dispose(&out);

	return filename;
}


// Source: icase.c
// Lines 46-73
