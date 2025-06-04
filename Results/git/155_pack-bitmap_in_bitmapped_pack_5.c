static int in_bitmapped_pack(struct bitmap_index *bitmap_git,
			     struct object_list *roots)
{
	while (roots) {
		struct object *object = roots->item;
		roots = roots->next;

		if (bitmap_is_midx(bitmap_git)) {
			if (bsearch_midx(&object->oid, bitmap_git->midx, NULL))
				return 1;
		} else {
			if (find_pack_entry_one(object->oid.hash, bitmap_git->pack) > 0)
				return 1;
		}
	}


// Source: pack-bitmap.c
// Lines 940-954
