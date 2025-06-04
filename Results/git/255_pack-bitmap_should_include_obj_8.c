static int should_include_obj(struct object *obj, void *_data)
{
	struct include_data *data = _data;
	int bitmap_pos;

	bitmap_pos = bitmap_position(data->bitmap_git, &obj->oid);
	if (bitmap_pos < 0)
		return 1;
	if ((data->seen && bitmap_get(data->seen, bitmap_pos)) ||
	     bitmap_get(data->base, bitmap_pos)) {
		obj->flags |= SEEN;
		return 0;
	}
	return 1;
}


// Source: pack-bitmap.c
// Lines 697-711
