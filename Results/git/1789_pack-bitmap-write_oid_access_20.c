static const struct object_id *oid_access(size_t pos, const void *table)
{
	const struct pack_idx_entry * const *index = table;
	return &index[pos]->oid;
}


// Source: pack-bitmap-write.c
// Lines 644-648
