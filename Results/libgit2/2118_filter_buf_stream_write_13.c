static int buf_stream_write(
	git_writestream *s, const char *buffer, size_t len)
{
	struct buf_stream *buf_stream = (struct buf_stream *)s;
	GIT_ASSERT_ARG(buf_stream);
	GIT_ASSERT(buf_stream->complete == 0);

	return git_str_put(buf_stream->target, buffer, len);
}


// Source: filter.c
// Lines 719-727
