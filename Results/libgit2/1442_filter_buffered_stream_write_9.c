static int buffered_stream_write(
	git_writestream *s, const char *buffer, size_t len)
{
	struct buffered_stream *buffered_stream = (struct buffered_stream *)s;
	GIT_ASSERT_ARG(buffered_stream);

	return git_str_put(&buffered_stream->input, buffer, len);
}


// Source: filter.c
// Lines 887-894
