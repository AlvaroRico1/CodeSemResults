static int unpack_loose_header_to_strbuf(git_zstream *stream, unsigned char *map,
					 unsigned long mapsize, void *buffer,
					 unsigned long bufsiz, struct strbuf *header)
{
	int status;

	status = unpack_loose_short_header(stream, map, mapsize, buffer, bufsiz);
	if (status < Z_OK)
		return -1;

	/*
	 * Check if entire header is unpacked in the first iteration.
	 */
	if (memchr(buffer, '\0', stream->next_out - (unsigned char *)buffer))
		return 0;

	/*
	 * buffer[0..bufsiz] was not large enough.  Copy the partial
	 * result out to header, and then append the result of further
	 * reading the stream.
	 */
	strbuf_add(header, buffer, stream->next_out - (unsigned char *)buffer);
	stream->next_out = buffer;
	stream->avail_out = bufsiz;

	do {
		obj_read_unlock();
		status = git_inflate(stream, 0);
		obj_read_lock();
		strbuf_add(header, buffer, stream->next_out - (unsigned char *)buffer);
		if (memchr(buffer, '\0', stream->next_out - (unsigned char *)buffer))
			return 0;
		stream->next_out = buffer;
		stream->avail_out = bufsiz;
	} while (status != Z_STREAM_END);


// Source: object-file.c
// Lines 1227-1261
