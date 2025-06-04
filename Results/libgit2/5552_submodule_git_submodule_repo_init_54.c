int git_submodule_repo_init(
	git_repository **out,
	const git_submodule *sm,
	int use_gitlink)
{
	int error;
	git_repository *sub_repo = NULL;
	const char *configured_url;
	git_config *cfg = NULL;
	git_str buf = GIT_STR_INIT;

	GIT_ASSERT_ARG(out);
	GIT_ASSERT_ARG(sm);

	/* get the configured remote url of the submodule */
	if ((error = git_str_printf(&buf, "submodule.%s.url", sm->name)) < 0 ||
		(error = git_repository_config_snapshot(&cfg, sm->repo)) < 0 ||
		(error = git_config_get_string(&configured_url, cfg, buf.ptr)) < 0 ||
		(error = submodule_repo_init(&sub_repo, sm->repo, sm->path, configured_url, use_gitlink)) < 0)
		goto done;

	*out = sub_repo;

done:
	git_config_free(cfg);
	git_str_dispose(&buf);
	return error;
}


// Source: submodule.c
// Lines 882-909
