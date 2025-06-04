int check_pack_crc(struct packed_git *p, struct pack_window **w_curs,
		   off_t offset, off_t len, unsigned int nr)
{
	const uint32_t *index_crc;
	uint32_t data_crc = crc32(0, NULL, 0);

	do {
		unsigned long avail;
		void *data = use_pack(p, w_curs, offset, &avail);
		if (avail > len)
			avail = len;
		data_crc = crc32(data_crc, data, avail);
		offset += avail;
		len -= avail;
	} while (len);

	index_crc = p->index_data;
	index_crc += 2 + 256 + (size_t)p->num_objects * (the_hash_algo->rawsz/4) + nr;

	return data_crc != ntohl(*index_crc);
}


// Source: pack-check.c
// Lines 25-45
