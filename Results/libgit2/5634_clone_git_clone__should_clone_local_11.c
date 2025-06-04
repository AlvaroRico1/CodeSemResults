int git_clone__should_clone_local(const char *url_or_path, git_clone_local_t local)
{
	git_str fromurl = GIT_STR_INIT;
	const char *path = url_or_path;
	bool is_url, is_local;

	if (local == GIT_CLONE_NO_LOCAL)
		return 0;

	if ((is_url = git_fs_path_is_local_file_url(url_or_path)) != 0) {
		if (git_fs_path_fromurl(&fromurl, url_or_path) < 0) {
			is_local = -1;
			goto done;
		}

		path = fromurl.ptr;
	}

	is_local = (!is_url || local != GIT_CLONE_LOCAL_AUTO) &&
		git_fs_path_isdir(path);

done:
	git_str_dispose(&fromurl);
	return is_local;
}


// Source: clone.c
// Lines 427-451
