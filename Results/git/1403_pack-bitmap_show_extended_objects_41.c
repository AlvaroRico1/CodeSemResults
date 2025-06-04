static void show_extended_objects(struct bitmap_index *bitmap_git,
				  struct rev_info *revs,
				  show_reachable_fn show_reach)
{
	struct bitmap *objects = bitmap_git->result;
	struct eindex *eindex = &bitmap_git->ext_index;
	uint32_t i;

	for (i = 0; i < eindex->count; ++i) {
		struct object *obj;

		if (!bitmap_get(objects, bitmap_num_objects(bitmap_git) + i))
			continue;

		obj = eindex->objects[i];
		if ((obj->type == OBJ_BLOB && !revs->blob_objects) ||
		    (obj->type == OBJ_TREE && !revs->tree_objects) ||
		    (obj->type == OBJ_TAG && !revs->tag_objects))
			continue;

		show_reach(&obj->oid, obj->type, 0, eindex->hashes[i], NULL, 0);
	}


// Source: pack-bitmap.c
// Lines 829-850
