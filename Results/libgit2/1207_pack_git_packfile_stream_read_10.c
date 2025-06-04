ssize_t git_packfile_stream_read(git_packfile_stream *obj, void *buffer, size_t len)
{
	unsigned int window_len;
	unsigned char *in;
	int error;

	if (obj->done)
		return 0;

	if ((in = pack_window_open(obj->p, &obj->mw, obj->curpos, &window_len)) == NULL)
		return GIT_EBUFS;

	if ((error = git_zstream_set_input(&obj->zstream, in, window_len)) < 0 ||
	    (error = git_zstream_get_output_chunk(buffer, &len, &obj->zstream)) < 0) {
		git_mwindow_close(&obj->mw);
		git_error_set(GIT_ERROR_ZLIB, "error reading from the zlib stream");
		return -1;
	}

	git_mwindow_close(&obj->mw);

	obj->curpos += window_len - obj->zstream.in_len;

	if (git_zstream_eos(&obj->zstream))
		obj->done = 1;

	/* If we didn't write anything out but we're not done, we need more data */
	if (!len && !git_zstream_eos(&obj->zstream))
		return GIT_EBUFS;

	return len;

}


// Source: pack.c
// Lines 848-880
