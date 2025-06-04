int git_blob_create_from_stream_commit(git_oid *out, git_writestream *_stream)
{
	int error;
	blob_writestream *stream = (blob_writestream *) _stream;

	/*
	 * We can make this more officient by avoiding writing to
	 * disk, but for now let's re-use the helper functions we
	 * have.
	 */
	if ((error = git_filebuf_flush(&stream->fbuf)) < 0)
		goto cleanup;

	error = git_blob__create_from_paths(out, NULL, stream->repo, stream->fbuf.path_lock,
					    stream->hintpath, 0, !!stream->hintpath);

cleanup:
	blob_writestream_free(_stream);
	return error;

}


// Source: blob.c
// Lines 371-391
