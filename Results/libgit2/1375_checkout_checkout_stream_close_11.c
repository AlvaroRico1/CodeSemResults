static int checkout_stream_close(git_writestream *s)
{
	struct checkout_stream *stream = (struct checkout_stream *)s;

	GIT_ASSERT_ARG(stream);
	GIT_ASSERT_ARG(stream->open);

	stream->open = 0;
	return p_close(stream->fd);
}


// Source: checkout.c
// Lines 1490-1499
