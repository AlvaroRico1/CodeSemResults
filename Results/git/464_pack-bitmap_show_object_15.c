static void show_object(struct object *object, const char *name, void *data_)
{
	struct bitmap_show_data *data = data_;
	int bitmap_pos;

	bitmap_pos = bitmap_position(data->bitmap_git, &object->oid);

	if (bitmap_pos < 0)
		bitmap_pos = ext_index_add_object(data->bitmap_git, object,
						  name);

	bitmap_set(data->base, bitmap_pos);
}


// Source: pack-bitmap.c
// Lines 631-643
