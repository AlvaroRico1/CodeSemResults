static void fake_wstream__free(git_odb_stream *_stream)
{
	fake_wstream *stream = (fake_wstream *)_stream;

	git__free(stream->buffer);
	git__free(stream);
}


// Source: odb.c
// Lines 384-390
