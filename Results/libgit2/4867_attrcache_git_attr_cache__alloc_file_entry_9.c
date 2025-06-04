int git_attr_cache__alloc_file_entry(
	git_attr_file_entry **out,
	git_repository *repo,
	const char *base,
	const char *path,
	git_pool *pool)
{
	git_str fullpath_str = GIT_STR_INIT;
	size_t baselen = 0, pathlen = strlen(path);
	size_t cachesize = sizeof(git_attr_file_entry) + pathlen + 1;
	git_attr_file_entry *ce;

	if (base != NULL && git_fs_path_root(path) < 0) {
		baselen = strlen(base);
		cachesize += baselen;

		if (baselen && base[baselen - 1] != '/')
			cachesize++;
	}

	ce = git_pool_mallocz(pool, cachesize);
	GIT_ERROR_CHECK_ALLOC(ce);

	if (baselen) {
		memcpy(ce->fullpath, base, baselen);

		if (base[baselen - 1] != '/')
			ce->fullpath[baselen++] = '/';
	}
	memcpy(&ce->fullpath[baselen], path, pathlen);

	fullpath_str.ptr = ce->fullpath;
	fullpath_str.size = pathlen + baselen;

	if (git_path_validate_str_length(repo, &fullpath_str) < 0)
		return -1;

	ce->path = &ce->fullpath[baselen];
	*out = ce;

	return 0;
}


// Source: attrcache.c
// Lines 40-81
