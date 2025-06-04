static int mailmap_add_blob(
	git_mailmap *mm, git_repository *repo, const char *rev)
{
	git_object *object = NULL;
	git_blob *blob = NULL;
	git_str content = GIT_STR_INIT;
	int error;

	GIT_ASSERT_ARG(mm);
	GIT_ASSERT_ARG(repo);

	error = git_revparse_single(&object, repo, rev);
	if (error < 0)
		goto cleanup;

	error = git_object_peel((git_object **)&blob, object, GIT_OBJECT_BLOB);
	if (error < 0)
		goto cleanup;

	error = git_blob__getbuf(&content, blob);
	if (error < 0)
		goto cleanup;

	error = mailmap_add_buffer(mm, content.ptr, content.size);
	if (error < 0)
		goto cleanup;

cleanup:
	git_str_dispose(&content);
	git_blob_free(blob);
	git_object_free(object);
	return error;
}


// Source: mailmap.c
// Lines 289-321
