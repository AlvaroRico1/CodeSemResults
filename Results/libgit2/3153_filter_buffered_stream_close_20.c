static int buffered_stream_close(git_writestream *s)
{
	struct buffered_stream *buffered_stream = (struct buffered_stream *)s;
	git_str *writebuf;
	git_error_state error_state = {0};
	int error;

	GIT_ASSERT_ARG(buffered_stream);

	error = buffered_stream->write_fn(
		buffered_stream->filter,
		buffered_stream->payload,
		buffered_stream->output,
		&buffered_stream->input,
		buffered_stream->source);

	if (error == GIT_PASSTHROUGH) {
		writebuf = &buffered_stream->input;
	} else if (error == 0) {
		writebuf = buffered_stream->output;
	} else {
		/* close stream before erroring out taking care
		 * to preserve the original error */
		git_error_state_capture(&error_state, error);
		buffered_stream->target->close(buffered_stream->target);
		git_error_state_restore(&error_state);
		return error;
	}

	if ((error = buffered_stream->target->write(
			buffered_stream->target, writebuf->ptr, writebuf->size)) == 0)
		error = buffered_stream->target->close(buffered_stream->target);

	return error;
}


// Source: filter.c
// Lines 896-930
