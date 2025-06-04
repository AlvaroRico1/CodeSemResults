int git_revparse_single(git_object **out, git_repository *repo, const char *spec)
{
	int error;
	git_object *obj = NULL;
	git_reference *ref = NULL;

	*out = NULL;

	if ((error = git_revparse_ext(&obj, &ref, repo, spec)) < 0)
		goto cleanup;

	git_reference_free(ref);

	*out = obj;

	return 0;

cleanup:
	git_object_free(obj);
	git_reference_free(ref);
	return error;
}


// Source: revparse.c
// Lines 870-891
