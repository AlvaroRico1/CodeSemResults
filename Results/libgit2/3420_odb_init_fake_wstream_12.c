static int init_fake_wstream(git_odb_stream **stream_p, git_odb_backend *backend, git_object_size_t size, git_object_t type)
{
	fake_wstream *stream;
	size_t blobsize;

	GIT_ERROR_CHECK_BLOBSIZE(size);
	blobsize = (size_t)size;

	stream = git__calloc(1, sizeof(fake_wstream));
	GIT_ERROR_CHECK_ALLOC(stream);

	stream->size = blobsize;
	stream->type = type;
	stream->buffer = git__malloc(blobsize);
	if (stream->buffer == NULL) {
		git__free(stream);
		return -1;
	}

	stream->stream.backend = backend;
	stream->stream.read = NULL; /* read only */
	stream->stream.write = &fake_wstream__write;
	stream->stream.finalize_write = &fake_wstream__fwrite;
	stream->stream.free = &fake_wstream__free;
	stream->stream.mode = GIT_STREAM_WRONLY;

	*stream_p = (git_odb_stream *)stream;
	return 0;
}


// Source: odb.c
// Lines 392-420
