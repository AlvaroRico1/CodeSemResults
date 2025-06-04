static int update_header_and_rehash(git_indexer *idx, git_indexer_progress *stats)
{
	void *ptr;
	size_t chunk = 1024*1024;
	off64_t hashed = 0;
	git_mwindow *w = NULL;
	git_mwindow_file *mwf;
	unsigned int left;

	mwf = &idx->pack->mwf;

	git_hash_init(&idx->trailer);


	/* Update the header to include the number of local objects we injected */
	idx->hdr.hdr_entries = htonl(stats->total_objects + stats->local_objects);
	if (write_at(idx, &idx->hdr, 0, sizeof(struct git_pack_header)) < 0)
		return -1;

	/*
	 * We now use the same technique as before to determine the
	 * hash. We keep reading up to the end and let
	 * hash_partially() keep the existing trailer out of the
	 * calculation.
	 */
	if (git_mwindow_free_all(mwf) < 0)
		return -1;

	idx->inbuf_len = 0;
	while (hashed < mwf->size) {
		ptr = git_mwindow_open(mwf, &w, hashed, chunk, &left);
		if (ptr == NULL)
			return -1;

		hash_partially(idx, ptr, left);
		hashed += left;

		git_mwindow_close(&w);
	}

	return 0;
}


// Source: indexer.c
// Lines 1120-1161
