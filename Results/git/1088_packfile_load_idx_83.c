int load_idx(const char *path, const unsigned int hashsz, void *idx_map,
	     size_t idx_size, struct packed_git *p)
{
	struct pack_idx_header *hdr = idx_map;
	uint32_t version, nr, i, *index;

	if (idx_size < 4 * 256 + hashsz + hashsz)
		return error("index file %s is too small", path);
	if (idx_map == NULL)
		return error("empty data");

	if (hdr->idx_signature == htonl(PACK_IDX_SIGNATURE)) {
		version = ntohl(hdr->idx_version);
		if (version < 2 || version > 2)
			return error("index file %s is version %"PRIu32
				     " and is not supported by this binary"
				     " (try upgrading GIT to a newer version)",
				     path, version);
	} else
		version = 1;

	nr = 0;
	index = idx_map;
	if (version > 1)
		index += 2;  /* skip index header */
	for (i = 0; i < 256; i++) {
		uint32_t n = ntohl(index[i]);
		if (n < nr)
			return error("non-monotonic index %s", path);
		nr = n;
	}

	if (version == 1) {
		/*
		 * Total size:
		 *  - 256 index entries 4 bytes each
		 *  - 24-byte entries * nr (object ID + 4-byte offset)
		 *  - hash of the packfile
		 *  - file checksum
		 */
		if (idx_size != st_add(4 * 256 + hashsz + hashsz, st_mult(nr, hashsz + 4)))
			return error("wrong index v1 file size in %s", path);
	} else if (version == 2) {
		/*
		 * Minimum size:
		 *  - 8 bytes of header
		 *  - 256 index entries 4 bytes each
		 *  - object ID entry * nr
		 *  - 4-byte crc entry * nr
		 *  - 4-byte offset entry * nr
		 *  - hash of the packfile
		 *  - file checksum
		 * And after the 4-byte offset table might be a
		 * variable sized table containing 8-byte entries
		 * for offsets larger than 2^31.
		 */
		size_t min_size = st_add(8 + 4*256 + hashsz + hashsz, st_mult(nr, hashsz + 4 + 4));
		size_t max_size = min_size;
		if (nr)
			max_size = st_add(max_size, st_mult(nr - 1, 8));
		if (idx_size < min_size || idx_size > max_size)
			return error("wrong index v2 file size in %s", path);
		if (idx_size != min_size &&
		    /*
		     * make sure we can deal with large pack offsets.
		     * 31-bit signed offset won't be enough, neither
		     * 32-bit unsigned one will be.
		     */
		    (sizeof(off_t) <= 4))
			return error("pack too large for current definition of off_t in %s", path);
		p->crc_offset = 8 + 4 * 256 + nr * hashsz;
	}

	p->index_version = version;
	p->index_data = idx_map;
	p->index_size = idx_size;
	p->num_objects = nr;
	return 0;
}


// Source: packfile.c
// Lines 111-189
