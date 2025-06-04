off_t find_pack_entry_one(const unsigned char *sha1,
				  struct packed_git *p)
{
	const unsigned char *index = p->index_data;
	struct object_id oid;
	uint32_t result;

	if (!index) {
		if (open_pack_index(p))
			return 0;
	}

	hashcpy(oid.hash, sha1);
	if (bsearch_pack(&oid, p, &result))
		return nth_packed_object_offset(p, result);
	return 0;
}


// Source: packfile.c
// Lines 1950-1966
