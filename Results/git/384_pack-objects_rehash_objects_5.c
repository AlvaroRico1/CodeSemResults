static void rehash_objects(struct packing_data *pdata)
{
	uint32_t i;
	struct object_entry *entry;

	pdata->index_size = closest_pow2(pdata->nr_objects * 3);
	if (pdata->index_size < 1024)
		pdata->index_size = 1024;

	free(pdata->index);
	CALLOC_ARRAY(pdata->index, pdata->index_size);

	entry = pdata->objects;

	for (i = 0; i < pdata->nr_objects; i++) {
		int found;
		uint32_t ix = locate_object_entry_hash(pdata,
						       &entry->idx.oid,
						       &found);

		if (found)
			BUG("Duplicate object in hash");

		pdata->index[ix] = i + 1;
		entry++;
	}
}


// Source: pack-objects.c
// Lines 42-68
