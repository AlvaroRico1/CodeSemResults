int git_signature_default(git_signature **out, git_repository *repo)
{
	int error;
	git_config *cfg;
	const char *user_name, *user_email;

	if ((error = git_repository_config_snapshot(&cfg, repo)) < 0)
		return error;

	if (!(error = git_config_get_string(&user_name, cfg, "user.name")) &&
		!(error = git_config_get_string(&user_email, cfg, "user.email")))
		error = git_signature_now(out, user_name, user_email);

	git_config_free(cfg);
	return error;
}


// Source: signature.c
// Lines 188-203
