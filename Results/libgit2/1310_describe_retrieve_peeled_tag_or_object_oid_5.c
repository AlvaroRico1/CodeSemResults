static int retrieve_peeled_tag_or_object_oid(
	git_oid *peeled_out,
	git_oid *ref_target_out,
	git_repository *repo,
	const char *refname)
{
	git_reference *ref;
	git_object *peeled = NULL;
	int error;

	if ((error = git_reference_lookup_resolved(&ref, repo, refname, -1)) < 0)
		return error;

	if ((error = git_reference_peel(&peeled, ref, GIT_OBJECT_ANY)) < 0)
		goto cleanup;

	git_oid_cpy(ref_target_out, git_reference_target(ref));
	git_oid_cpy(peeled_out, git_object_id(peeled));

	if (git_oid_cmp(ref_target_out, peeled_out) != 0)
		error = 1; /* The reference was pointing to a annotated tag */
	else
		error = 0; /* Any other object */

cleanup:
	git_reference_free(ref);
	git_object_free(peeled);
	return error;
}


// Source: describe.c
// Lines 133-161
