static int fake_wstream__write(git_odb_stream *_stream, const char *data, size_t len)
{
	fake_wstream *stream = (fake_wstream *)_stream;

	GIT_ASSERT(stream->written + len <= stream->size);

	memcpy(stream->buffer + stream->written, data, len);
	stream->written += len;
	return 0;
}


// Source: odb.c
// Lines 373-382
