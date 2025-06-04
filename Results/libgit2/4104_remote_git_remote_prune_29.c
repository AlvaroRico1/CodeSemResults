int git_remote_prune(git_remote *remote, const git_remote_callbacks *callbacks)
{
	size_t i, j;
	git_vector remote_refs = GIT_VECTOR_INIT;
	git_vector candidates = GIT_VECTOR_INIT;
	const git_refspec *spec;
	const char *refname;
	int error;
	git_oid zero_id = {{ 0 }};

	if (callbacks)
		GIT_ERROR_CHECK_VERSION(callbacks, GIT_REMOTE_CALLBACKS_VERSION, "git_remote_callbacks");

	if ((error = ls_to_vector(&remote_refs, remote)) < 0)
		goto cleanup;

	git_vector_set_cmp(&remote_refs, find_head);

	if ((error = prune_candidates(&candidates, remote)) < 0)
		goto cleanup;

	/*
	 * Remove those entries from the candidate list for which we
	 * can find a remote reference in at least one refspec.
	 */
	git_vector_foreach(&candidates, i, refname) {
		git_vector_foreach(&remote->active_refspecs, j, spec) {
			git_str buf = GIT_STR_INIT;
			size_t pos;
			char *src_name;
			git_remote_head key = {0};

			if (!git_refspec_dst_matches(spec, refname))
				continue;

			if ((error = git_refspec__rtransform(&buf, spec, refname)) < 0)
				goto cleanup;

			key.name = (char *) git_str_cstr(&buf);
			error = git_vector_bsearch(&pos, &remote_refs, &key);
			git_str_dispose(&buf);

			if (error < 0 && error != GIT_ENOTFOUND)
				goto cleanup;

			if (error == GIT_ENOTFOUND)
				continue;

			/* If we did find a source, remove it from the candidates. */
			if ((error = git_vector_set((void **) &src_name, &candidates, i, NULL)) < 0)
				goto cleanup;

			git__free(src_name);
			break;
		}
	}

	/*
	 * For those candidates still left in the list, we need to
	 * remove them. We do not remove symrefs, as those are for
	 * stuff like origin/HEAD which will never match, but we do
	 * not want to remove them.
	 */
	git_vector_foreach(&candidates, i, refname) {
		git_reference *ref;
		git_oid id;

		if (refname == NULL)
			continue;

		error = git_reference_lookup(&ref, remote->repo, refname);
		/* as we want it gone, let's not consider this an error */
		if (error == GIT_ENOTFOUND)
			continue;

		if (error < 0)
			goto cleanup;

		if (git_reference_type(ref) == GIT_REFERENCE_SYMBOLIC) {
			git_reference_free(ref);
			continue;
		}

		git_oid_cpy(&id, git_reference_target(ref));
		error = git_reference_delete(ref);
		git_reference_free(ref);
		if (error < 0)
			goto cleanup;

		if (callbacks && callbacks->update_tips)
			error = callbacks->update_tips(refname, &id, &zero_id, callbacks->payload);

		if (error < 0)
			goto cleanup;
	}


// Source: remote.c
// Lines 1617-1711
