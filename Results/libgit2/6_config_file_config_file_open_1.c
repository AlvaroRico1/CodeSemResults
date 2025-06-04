static int config_file_open(git_config_backend *cfg, git_config_level_t level, const git_repository *repo)
{
	config_file_backend *b = GIT_CONTAINER_OF(cfg, config_file_backend, parent);
	int res;

	b->level = level;
	b->repo = repo;

	if ((res = git_config_entries_new(&b->entries)) < 0)
		return res;

	if (!git_fs_path_exists(b->file.path))
		return 0;

	/*
	 * git silently ignores configuration files that are not
	 * readable.  We emulate that behavior.  This is particularly
	 * important for sandboxed applications on macOS where the
	 * git configuration files may not be readable.
	 */
	if (p_access(b->file.path, R_OK) < 0)
		return GIT_ENOTFOUND;

	if (res < 0 || (res = config_file_read(b->entries, repo, &b->file, level, 0)) < 0) {
		git_config_entries_free(b->entries);
		b->entries = NULL;
	}

	return res;
}


// Source: config_file.c
// Lines 101-130
