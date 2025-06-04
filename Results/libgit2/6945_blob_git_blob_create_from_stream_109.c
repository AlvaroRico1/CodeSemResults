int git_blob_create_from_stream(git_writestream **out, git_repository *repo, const char *hintpath)
{
	int error;
	git_str path = GIT_STR_INIT;
	blob_writestream *stream;

	GIT_ASSERT_ARG(out);
	GIT_ASSERT_ARG(repo);

	stream = git__calloc(1, sizeof(blob_writestream));
	GIT_ERROR_CHECK_ALLOC(stream);

	if (hintpath) {
		stream->hintpath = git__strdup(hintpath);
		GIT_ERROR_CHECK_ALLOC(stream->hintpath);
	}

	stream->repo = repo;
	stream->parent.write = blob_writestream_write;
	stream->parent.close = blob_writestream_close;
	stream->parent.free  = blob_writestream_free;

	if ((error = git_repository__item_path(&path, repo, GIT_REPOSITORY_ITEM_OBJECTS)) < 0
		|| (error = git_str_joinpath(&path, path.ptr, "streamed")) < 0)
		goto cleanup;

	if ((error = git_filebuf_open_withsize(&stream->fbuf, git_str_cstr(&path), GIT_FILEBUF_TEMPORARY,
					       0666, 2 * 1024 * 1024)) < 0)
		goto cleanup;

	*out = (git_writestream *) stream;

cleanup:
	if (error < 0)
		blob_writestream_free((git_writestream *) stream);

	git_str_dispose(&path);
	return error;
}


// Source: blob.c
// Lines 331-369
