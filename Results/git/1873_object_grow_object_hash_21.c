static void grow_object_hash(struct repository *r)
{
	int i;
	/*
	 * Note that this size must always be power-of-2 to match hash_obj
	 * above.
	 */
	int new_hash_size = r->parsed_objects->obj_hash_size < 32 ? 32 : 2 * r->parsed_objects->obj_hash_size;
	struct object **new_hash;

	CALLOC_ARRAY(new_hash, new_hash_size);
	for (i = 0; i < r->parsed_objects->obj_hash_size; i++) {
		struct object *obj = r->parsed_objects->obj_hash[i];

		if (!obj)
			continue;
		insert_obj_hash(obj, new_hash, new_hash_size);
	}
	free(r->parsed_objects->obj_hash);
	r->parsed_objects->obj_hash = new_hash;
	r->parsed_objects->obj_hash_size = new_hash_size;
}


// Source: object.c
// Lines 120-141
