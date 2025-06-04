static struct bitmap *find_tip_objects(struct bitmap_index *bitmap_git,
				       struct object_list *tip_objects,
				       enum object_type type)
{
	struct bitmap *result = bitmap_new();
	struct object_list *p;

	for (p = tip_objects; p; p = p->next) {
		int pos;

		if (p->item->type != type)
			continue;

		pos = bitmap_position(bitmap_git, &p->item->oid);
		if (pos < 0)
			continue;

		bitmap_set(result, pos);
	}

	return result;
}


// Source: pack-bitmap.c
// Lines 959-980
