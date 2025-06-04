static int remote_head_for_ref(git_remote_head **out, git_remote *remote, git_refspec *spec, git_vector *update_heads, git_reference *ref)
{
	git_reference *resolved_ref = NULL;
	git_str remote_name = GIT_STR_INIT;
	git_config *config = NULL;
	const char *ref_name;
	int error = 0, update;

	GIT_ASSERT_ARG(out);
	GIT_ASSERT_ARG(spec);
	GIT_ASSERT_ARG(ref);

	*out = NULL;

	error = git_reference_resolve(&resolved_ref, ref);

	/* If we're in an unborn branch, let's pretend nothing happened */
	if (error == GIT_ENOTFOUND && git_reference_type(ref) == GIT_REFERENCE_SYMBOLIC) {
		ref_name = git_reference_symbolic_target(ref);
		error = 0;
	} else {
		ref_name = git_reference_name(resolved_ref);
	}

	/*
	 * The ref name may be unresolvable - perhaps it's pointing to
	 * something invalid.  In this case, there is no remote head for
	 * this ref.
	 */
	if (!ref_name) {
		error = 0;
		goto cleanup;
	}

	if ((error = ref_to_update(&update, &remote_name, remote, spec, ref_name)) < 0)
		goto cleanup;

	if (update)
		error = remote_head_for_fetchspec_src(out, update_heads, git_str_cstr(&remote_name));

cleanup:
	git_str_dispose(&remote_name);
	git_reference_free(resolved_ref);
	git_config_free(config);
	return error;
}


// Source: remote.c
// Lines 1465-1510
