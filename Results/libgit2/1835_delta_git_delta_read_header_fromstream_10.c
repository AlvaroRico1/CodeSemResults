int git_delta_read_header_fromstream(
	size_t *base_sz, size_t *res_sz, git_packfile_stream *stream)
{
	static const size_t buffer_len = DELTA_HEADER_BUFFER_LEN;
	unsigned char buffer[DELTA_HEADER_BUFFER_LEN];
	const unsigned char *delta, *delta_end;
	size_t len;
	ssize_t read;

	len = read = 0;
	while (len < buffer_len) {
		read = git_packfile_stream_read(stream, &buffer[len], buffer_len - len);

		if (read == 0)
			break;

		if (read == GIT_EBUFS)
			continue;

		len += read;
	}

	delta = buffer;
	delta_end = delta + len;
	if ((hdr_sz(base_sz, &delta, delta_end) < 0) ||
		(hdr_sz(res_sz, &delta, delta_end) < 0))
		return -1;

	return 0;
}


// Source: delta.c
// Lines 503-532
