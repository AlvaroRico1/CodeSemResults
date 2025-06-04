static int write_at(git_indexer *idx, const void *data, off64_t offset, size_t size)
{
	size_t remaining_size = size;
	const char *ptr = (const char *)data;

	/* Handle data size larger that ssize_t */
	while (remaining_size > 0) {
		ssize_t nb;
		HANDLE_EINTR(nb, p_pwrite(idx->pack->mwf.fd, (void *)ptr,
					  remaining_size, offset));
		if (nb <= 0)
			return -1;

		ptr += nb;
		offset += nb;
		remaining_size -= nb;
	}

	return 0;
}


// Source: indexer.c
// Lines 620-639
