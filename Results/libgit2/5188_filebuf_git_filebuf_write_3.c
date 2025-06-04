int git_filebuf_write(git_filebuf *file, const void *buff, size_t len)
{
	const unsigned char *buf = buff;

	ENSURE_BUF_OK(file);

	if (file->do_not_buffer)
		return file->write(file, (void *)buff, len);

	for (;;) {
		size_t space_left = file->buf_size - file->buf_pos;

		/* cache if it's small */
		if (space_left > len) {
			add_to_cache(file, buf, len);
			return 0;
		}

		add_to_cache(file, buf, space_left);
		if (flush_buffer(file) < 0)
			return -1;

		len -= space_left;
		buf += space_left;
	}
}


// Source: filebuf.c
// Lines 466-491
