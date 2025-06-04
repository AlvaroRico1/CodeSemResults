static void blob_writestream_free(git_writestream *_stream)
{
	blob_writestream *stream = (blob_writestream *) _stream;

	git_filebuf_cleanup(&stream->fbuf);
	git__free(stream->hintpath);
	git__free(stream);
}


// Source: blob.c
// Lines 315-322
