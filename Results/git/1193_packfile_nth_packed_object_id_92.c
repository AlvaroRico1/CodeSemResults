int nth_packed_object_id(struct object_id *oid,
			 struct packed_git *p,
			 uint32_t n)
{
	const unsigned char *index = p->index_data;
	const unsigned int hashsz = the_hash_algo->rawsz;
	if (!index) {
		if (open_pack_index(p))
			return -1;
		index = p->index_data;
	}
	if (n >= p->num_objects)
		return -1;
	index += 4 * 256;
	if (p->index_version == 1) {
		oidread(oid, index + (hashsz + 4) * n + 4);
	} else {
		index += 8;
		oidread(oid, index + hashsz * n);
	}
	return 0;
}


// Source: packfile.c
// Lines 1894-1915
