int finalize_hashfile(struct hashfile *f, unsigned char *result, unsigned int flags)
{
	int fd;

	hashflush(f);
	the_hash_algo->final_fn(f->buffer, &f->ctx);
	if (result)
		hashcpy(result, f->buffer);
	if (flags & CSUM_HASH_IN_STREAM)
		flush(f, f->buffer, the_hash_algo->rawsz);
	if (flags & CSUM_FSYNC)
		fsync_or_die(f->fd, f->name);
	if (flags & CSUM_CLOSE) {
		if (close(f->fd))
			die_errno("%s: sha1 file error on close", f->name);
		fd = 0;
	} else
		fd = f->fd;
	if (0 <= f->check_fd) {
		char discard;
		int cnt = read_in_full(f->check_fd, &discard, 1);
		if (cnt < 0)
			die_errno("%s: error when reading the tail of sha1 file",
				  f->name);
		if (cnt)
			die("%s: sha1 file has trailing garbage", f->name);
		if (close(f->check_fd))
			die_errno("%s: sha1 file error on close", f->name);
	}


// Source: csum-file.c
// Lines 61-89
