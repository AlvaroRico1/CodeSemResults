static size_t pipe_zero(size_t bytes, struct iov_iter *i)
{
	struct pipe_inode_info *pipe = i->pipe;
	size_t n, off;
	int idx;

	if (!sanity(i))
		return 0;

	bytes = n = push_pipe(i, bytes, &idx, &off);
	if (unlikely(!n))
		return 0;

	for ( ; n; idx = next_idx(idx, pipe), off = 0) {
		size_t chunk = min_t(size_t, n, PAGE_SIZE - off);
		memzero_page(pipe->bufs[idx].page, off, chunk);
		i->idx = idx;
		i->iov_offset = off + chunk;
		n -= chunk;
	}
	i->count -= bytes;
	return bytes;
}


// Source: iov_iter.c
// Lines 925-947
