static void filter_bitmap_object_type(struct bitmap_index *bitmap_git,
				      struct object_list *tip_objects,
				      struct bitmap *to_filter,
				      enum object_type object_type)
{
	if (object_type < OBJ_COMMIT || object_type > OBJ_TAG)
		BUG("filter_bitmap_object_type given invalid object");

	if (object_type != OBJ_TAG)
		filter_bitmap_exclude_type(bitmap_git, tip_objects, to_filter, OBJ_TAG);
	if (object_type != OBJ_COMMIT)
		filter_bitmap_exclude_type(bitmap_git, tip_objects, to_filter, OBJ_COMMIT);
	if (object_type != OBJ_TREE)
		filter_bitmap_exclude_type(bitmap_git, tip_objects, to_filter, OBJ_TREE);
	if (object_type != OBJ_BLOB)
		filter_bitmap_exclude_type(bitmap_git, tip_objects, to_filter, OBJ_BLOB);
}


// Source: pack-bitmap.c
// Lines 1135-1151
