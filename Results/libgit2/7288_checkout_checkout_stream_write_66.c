static int checkout_stream_write(
	git_writestream *s, const char *buffer, size_t len)
{
	struct checkout_stream *stream = (struct checkout_stream *)s;
	int ret;

	if ((ret = p_write(stream->fd, buffer, len)) < 0)
		git_error_set(GIT_ERROR_OS, "could not write to '%s'", stream->path);

	return ret;
}


// Source: checkout.c
// Lines 1478-1488
