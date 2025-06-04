off_t nth_packed_object_offset(const struct packed_git *p, uint32_t n)
{
	const unsigned char *index = p->index_data;
	const unsigned int hashsz = the_hash_algo->rawsz;
	index += 4 * 256;
	if (p->index_version == 1) {
		return ntohl(*((uint32_t *)(index + (hashsz + 4) * (size_t)n)));
	} else {
		uint32_t off;
		index += 8 + (size_t)p->num_objects * (hashsz + 4);
		off = ntohl(*((uint32_t *)(index + 4 * n)));
		if (!(off & 0x80000000))
			return off;
		index += (size_t)p->num_objects * 4 + (off & 0x7fffffff) * 8;
		check_pack_index_ptr(p, index);
		return get_be64(index);
	}
}


// Source: packfile.c
// Lines 1931-1948
