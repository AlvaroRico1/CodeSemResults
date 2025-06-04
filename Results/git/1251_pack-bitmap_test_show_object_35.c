static void test_show_object(struct object *object, const char *name,
			     void *data)
{
	struct bitmap_test_data *tdata = data;
	int bitmap_pos;

	bitmap_pos = bitmap_position(tdata->bitmap_git, &object->oid);
	if (bitmap_pos < 0)
		die("Object not in bitmap: %s\n", oid_to_hex(&object->oid));
	test_bitmap_type(tdata, object, bitmap_pos);

	bitmap_set(tdata->base, bitmap_pos);
	display_progress(tdata->prg, ++tdata->seen);
}


// Source: pack-bitmap.c
// Lines 1636-1649
