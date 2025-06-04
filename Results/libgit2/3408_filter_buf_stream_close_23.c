static int buf_stream_close(git_writestream *s)
{
	struct buf_stream *buf_stream = (struct buf_stream *)s;
	GIT_ASSERT_ARG(buf_stream);

	GIT_ASSERT(buf_stream->complete == 0);
	buf_stream->complete = 1;

	return 0;
}


// Source: filter.c
// Lines 729-738
