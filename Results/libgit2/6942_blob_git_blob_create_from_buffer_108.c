int git_blob_create_from_buffer(
	git_oid *id, git_repository *repo, const void *buffer, size_t len)
{
	int error;
	git_odb *odb;
	git_odb_stream *stream;

	GIT_ASSERT_ARG(id);
	GIT_ASSERT_ARG(repo);

	if ((error = git_repository_odb__weakptr(&odb, repo)) < 0 ||
		(error = git_odb_open_wstream(&stream, odb, len, GIT_OBJECT_BLOB)) < 0)
		return error;

	if ((error = git_odb_stream_write(stream, buffer, len)) == 0)
		error = git_odb_stream_finalize_write(id, stream);

	git_odb_stream_free(stream);
	return error;
}


// Source: blob.c
// Lines 79-98
