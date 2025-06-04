static int attr_setup(
	git_repository *repo,
	git_attr_session *attr_session,
	git_attr_options *opts)
{
	git_str system = GIT_STR_INIT, info = GIT_STR_INIT;
	git_attr_file_source index_source = { GIT_ATTR_FILE_SOURCE_INDEX, NULL, GIT_ATTR_FILE, NULL };
	git_attr_file_source head_source = { GIT_ATTR_FILE_SOURCE_HEAD, NULL, GIT_ATTR_FILE, NULL };
	git_attr_file_source commit_source = { GIT_ATTR_FILE_SOURCE_COMMIT, NULL, GIT_ATTR_FILE, NULL };
	git_index *idx = NULL;
	const char *workdir;
	int error = 0;

	if (attr_session && attr_session->init_setup)
		return 0;

	if ((error = git_attr_cache__init(repo)) < 0)
		return error;

	/*
	 * Preload attribute files that could contain macros so the
	 * definitions will be available for later file parsing.
	 */

	if ((error = system_attr_file(&system, attr_session)) < 0 ||
	    (error = preload_attr_file(repo, attr_session, NULL, system.ptr)) < 0) {
		if (error != GIT_ENOTFOUND)
			goto out;

		error = 0;
	}

	if ((error = preload_attr_file(repo, attr_session, NULL,
	                               git_repository_attr_cache(repo)->cfg_attr_file)) < 0)
		goto out;

	if ((error = git_repository__item_path(&info, repo, GIT_REPOSITORY_ITEM_INFO)) < 0 ||
	    (error = preload_attr_file(repo, attr_session, info.ptr, GIT_ATTR_FILE_INREPO)) < 0) {
		if (error != GIT_ENOTFOUND)
			goto out;

		error = 0;
	}

	if ((workdir = git_repository_workdir(repo)) != NULL &&
	    (error = preload_attr_file(repo, attr_session, workdir, GIT_ATTR_FILE)) < 0)
			goto out;

	if ((error = git_repository_index__weakptr(&idx, repo)) < 0 ||
	    (error = preload_attr_source(repo, attr_session, &index_source)) < 0)
			goto out;

	if ((opts && (opts->flags & GIT_ATTR_CHECK_INCLUDE_HEAD) != 0) &&
	    (error = preload_attr_source(repo, attr_session, &head_source)) < 0)
		goto out;

	if ((opts && (opts->flags & GIT_ATTR_CHECK_INCLUDE_COMMIT) != 0)) {
#ifndef GIT_DEPRECATE_HARD
		if (opts->commit_id)
			commit_source.commit_id = opts->commit_id;
		else
#endif
		commit_source.commit_id = &opts->attr_commit_id;

		if ((error = preload_attr_source(repo, attr_session, &commit_source)) < 0)
			goto out;
	}

	if (attr_session)
		attr_session->init_setup = 1;

out:
	git_str_dispose(&system);
	git_str_dispose(&info);

	return error;
}


// Source: attr.c
// Lines 378-454
