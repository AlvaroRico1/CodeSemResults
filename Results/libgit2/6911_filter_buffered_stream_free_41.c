static void buffered_stream_free(git_writestream *s)
{
	struct buffered_stream *buffered_stream = (struct buffered_stream *)s;

	if (buffered_stream) {
		git_str_dispose(&buffered_stream->input);
		git_str_dispose(&buffered_stream->temp_buf);
		git__free(buffered_stream);
	}
}


// Source: filter.c
// Lines 932-941
