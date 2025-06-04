static int blob_writestream_write(git_writestream *_stream, const char *buffer, size_t len)
{
	blob_writestream *stream = (blob_writestream *) _stream;

	return git_filebuf_write(&stream->fbuf, buffer, len);
}


// Source: blob.c
// Lines 324-329
