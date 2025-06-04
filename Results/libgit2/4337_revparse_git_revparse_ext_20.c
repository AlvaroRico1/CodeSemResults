int git_revparse_ext(
	git_object **object_out,
	git_reference **reference_out,
	git_repository *repo,
	const char *spec)
{
	int error;
	size_t identifier_len;
	git_object *obj = NULL;
	git_reference *ref = NULL;

	if ((error = revparse(&obj, &ref, &identifier_len, repo, spec)) < 0)
		goto cleanup;

	*object_out = obj;
	*reference_out = ref;
	GIT_UNUSED(identifier_len);

	return 0;

cleanup:
	git_object_free(obj);
	git_reference_free(ref);
	return error;
}


// Source: revparse.c
// Lines 844-868
