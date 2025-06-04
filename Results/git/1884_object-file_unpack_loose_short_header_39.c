static int unpack_loose_short_header(git_zstream *stream,
				     unsigned char *map, unsigned long mapsize,
				     void *buffer, unsigned long bufsiz)
{
	int ret;

	/* Get the data stream */
	memset(stream, 0, sizeof(*stream));
	stream->next_in = map;
	stream->avail_in = mapsize;
	stream->next_out = buffer;
	stream->avail_out = bufsiz;

	git_inflate_init(stream);
	obj_read_unlock();
	ret = git_inflate(stream, 0);
	obj_read_lock();

	return ret;
}


// Source: object-file.c
// Lines 1190-1209
