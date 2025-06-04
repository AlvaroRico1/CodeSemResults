void repo_clear_commit_marks(struct repository *r, unsigned int flags)
{
	int i;

	for (i = 0; i < r->parsed_objects->obj_hash_size; i++) {
		struct object *obj = r->parsed_objects->obj_hash[i];
		if (obj && obj->type == OBJ_COMMIT)
			obj->flags &= ~flags;
	}


// Source: object.c
// Lines 475-483
