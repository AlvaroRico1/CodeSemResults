static int write_deflate(git_filebuf *file, void *source, size_t len)
{
	z_stream *zs = &file->zs;

	if (len > 0 || file->flush_mode == Z_FINISH) {
		zs->next_in = source;
		zs->avail_in = (uInt)len;

		do {
			size_t have;

			zs->next_out = file->z_buf;
			zs->avail_out = (uInt)file->buf_size;

			if (deflate(zs, file->flush_mode) == Z_STREAM_ERROR) {
				file->last_error = BUFERR_ZLIB;
				return -1;
			}

			have = file->buf_size - (size_t)zs->avail_out;

			if (p_write(file->fd, file->z_buf, have) < 0) {
				file->last_error = BUFERR_WRITE;
				return -1;
			}

		} while (zs->avail_out == 0);

		GIT_ASSERT(zs->avail_in == 0);

		if (file->compute_digest)
			git_hash_update(&file->digest, source, len);
	}

	return 0;
}


// Source: filebuf.c
// Lines 159-194
