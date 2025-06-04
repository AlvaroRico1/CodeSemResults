static uint32_t count_object_type(struct bitmap_index *bitmap_git,
				  enum object_type type)
{
	struct bitmap *objects = bitmap_git->result;
	struct eindex *eindex = &bitmap_git->ext_index;

	uint32_t i = 0, count = 0;
	struct ewah_iterator it;
	eword_t filter;

	init_type_iterator(&it, bitmap_git, type);

	while (i < objects->word_alloc && ewah_iterator_next(&filter, &it)) {
		eword_t word = objects->words[i++] & filter;
		count += ewah_bit_popcount64(word);
	}

	for (i = 0; i < eindex->count; ++i) {
		if (eindex->objects[i]->type == type &&
			bitmap_get(objects, bitmap_num_objects(bitmap_git) + i))
			count++;
	}

	return count;
}


// Source: pack-bitmap.c
// Lines 1542-1566
