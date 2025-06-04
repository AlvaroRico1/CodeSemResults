int git_branch__upstream_name(
	git_str *out,
	git_repository *repo,
	const char *refname)
{
	git_str remote_name = GIT_STR_INIT;
	git_str merge_name = GIT_STR_INIT;
	git_str buf = GIT_STR_INIT;
	int error = -1;
	git_remote *remote = NULL;
	const git_refspec *refspec;
	git_config *config;

	GIT_ASSERT_ARG(out);
	GIT_ASSERT_ARG(repo);
	GIT_ASSERT_ARG(refname);

	if (!git_reference__is_branch(refname))
		return not_a_local_branch(refname);

	if ((error = git_repository_config_snapshot(&config, repo)) < 0)
		return error;

	if ((error = retrieve_upstream_configuration(
		&remote_name, config, refname, "branch.%s.remote")) < 0)
			goto cleanup;

	if ((error = retrieve_upstream_configuration(
		&merge_name, config, refname, "branch.%s.merge")) < 0)
			goto cleanup;

	if (git_str_len(&remote_name) == 0 || git_str_len(&merge_name) == 0) {
		git_error_set(GIT_ERROR_REFERENCE,
			"branch '%s' does not have an upstream", refname);
		error = GIT_ENOTFOUND;
		goto cleanup;
	}

	if (strcmp(".", git_str_cstr(&remote_name)) != 0) {
		if ((error = git_remote_lookup(&remote, repo, git_str_cstr(&remote_name))) < 0)
			goto cleanup;

		refspec = git_remote__matching_refspec(remote, git_str_cstr(&merge_name));
		if (!refspec) {
			error = GIT_ENOTFOUND;
			goto cleanup;
		}

		if (git_refspec__transform(&buf, refspec, git_str_cstr(&merge_name)) < 0)
			goto cleanup;
	} else
		if (git_str_set(&buf, git_str_cstr(&merge_name), git_str_len(&merge_name)) < 0)
			goto cleanup;

	git_str_swap(out, &buf);

cleanup:
	git_config_free(config);
	git_remote_free(remote);
	git_str_dispose(&remote_name);
	git_str_dispose(&merge_name);
	git_str_dispose(&buf);
	return error;
}


// Source: branch.c
// Lines 416-479
