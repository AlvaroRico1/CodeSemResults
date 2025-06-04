static int recv_stream(gitno_buffer *buf)
{
	git_stream *io = (git_stream *) buf->cb_data;
	size_t readlen = buf->len - buf->offset;
	ssize_t ret;

	readlen = min(readlen, INT_MAX);

	ret = git_stream_read(io, buf->data + buf->offset, (int)readlen);
	if (ret < 0)
		return -1;

	buf->offset += ret;
	return (int)ret;
}


// Source: netops.c
// Lines 36-50
