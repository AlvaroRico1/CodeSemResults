static int blob_writestream_close(git_writestream *_stream)
{
	blob_writestream *stream = (blob_writestream *) _stream;

	git_filebuf_cleanup(&stream->fbuf);
	return 0;
}


// Source: blob.c
// Lines 307-313
