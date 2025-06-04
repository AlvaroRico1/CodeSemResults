static void write_hash_cache(struct hashfile *f,
			     struct pack_idx_entry **index,
			     uint32_t index_nr)
{
	uint32_t i;

	for (i = 0; i < index_nr; ++i) {
		struct object_entry *entry = (struct object_entry *)index[i];
		hashwrite_be32(f, entry->hash);
	}


// Source: pack-bitmap-write.c
// Lines 673-682
