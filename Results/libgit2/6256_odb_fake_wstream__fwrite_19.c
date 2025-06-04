static int fake_wstream__fwrite(git_odb_stream *_stream, const git_oid *oid)
{
	fake_wstream *stream = (fake_wstream *)_stream;
	return _stream->backend->write(_stream->backend, oid, stream->buffer, stream->size, stream->type);
}


// Source: odb.c
// Lines 367-371
