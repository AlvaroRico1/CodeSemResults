int git_reference__update_terminal(
	git_repository *repo,
	const char *ref_name,
	const git_oid *oid,
	const git_signature *sig,
	const char *log_message)
{
	git_reference *ref = NULL, *ref2 = NULL;
	git_signature *who = NULL;
	git_refdb *refdb = NULL;
	const git_signature *to_use;
	int error = 0;

	if (!sig && (error = git_reference__log_signature(&who, repo)) < 0)
		goto out;

	to_use = sig ? sig : who;

	if ((error = git_repository_refdb__weakptr(&refdb, repo)) < 0)
		goto out;

	if ((error = git_refdb_resolve(&ref, refdb, ref_name, -1)) < 0) {
		if (error == GIT_ENOTFOUND) {
			git_error_clear();
			error = reference__create(&ref2, repo, ref_name, oid, NULL, 0, to_use,
						  log_message, NULL, NULL);
		}
		goto out;
	}

	/* In case the resolved reference is symbolic, then it's a dangling symref. */
	if (git_reference_type(ref) == GIT_REFERENCE_SYMBOLIC) {
		error = reference__create(&ref2, repo, ref->target.symbolic, oid, NULL, 0, to_use,
					  log_message, NULL, NULL);
	} else {
		error = reference__create(&ref2, repo, ref->name, oid, NULL, 1, to_use,
					  log_message, &ref->target.oid, NULL);
	}

out:
	git_reference_free(ref2);
	git_reference_free(ref);
	git_signature_free(who);
	return error;
}


// Source: refs.c
// Lines 1079-1123
