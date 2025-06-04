static int push_one_attr(void *ref, const char *path)
{
	attr_walk_up_info *info = (attr_walk_up_info *)ref;
	git_attr_file_source_t src[GIT_ATTR_FILE_NUM_SOURCES];
	int error = 0, n_src, i;
	bool allow_macros;

	n_src = attr_decide_sources(info->opts ? info->opts->flags : 0,
	                            info->workdir != NULL,
	                            info->index != NULL,
	                            src);

	allow_macros = info->workdir ? !strcmp(info->workdir, path) : false;

	for (i = 0; !error && i < n_src; ++i) {
		git_attr_file_source source = { src[i], path, GIT_ATTR_FILE };

		if (src[i] == GIT_ATTR_FILE_SOURCE_COMMIT && info->opts) {
#ifndef GIT_DEPRECATE_HARD
			if (info->opts->commit_id)
				source.commit_id = info->opts->commit_id;
			else
#endif
			source.commit_id = &info->opts->attr_commit_id;
		}

		error = push_attr_source(info->repo, info->attr_session, info->files,
		                       &source, allow_macros);
	}


// Source: attr.c
// Lines 575-603
