static off_t get_disk_usage_for_extended(struct bitmap_index *bitmap_git)
{
	struct bitmap *result = bitmap_git->result;
	struct eindex *eindex = &bitmap_git->ext_index;
	off_t total = 0;
	struct object_info oi = OBJECT_INFO_INIT;
	off_t object_size;
	size_t i;

	oi.disk_sizep = &object_size;

	for (i = 0; i < eindex->count; i++) {
		struct object *obj = eindex->objects[i];

		if (!bitmap_get(result, bitmap_num_objects(bitmap_git) + i))
			continue;

		if (oid_object_info_extended(the_repository, &obj->oid, &oi, 0) < 0)
			die(_("unable to get disk usage of %s"),
			    oid_to_hex(&obj->oid));

		total += object_size;
	}


// Source: pack-bitmap.c
// Lines 1934-1956
