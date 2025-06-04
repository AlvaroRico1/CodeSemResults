static int handle_one_ref(const char *path, const struct object_id *oid,
			  int flag, void *cb_data)
{
	struct handle_one_ref_cb *cb = cb_data;
	struct commit_list **list = cb->list;
	struct object *object = parse_object(cb->repo, oid);
	if (!object)
		return 0;
	if (object->type == OBJ_TAG) {
		object = deref_tag(cb->repo, object, path,
				   strlen(path));
		if (!object)
			return 0;
	}
	if (object->type != OBJ_COMMIT)
		return 0;
	commit_list_insert((struct commit *)object, list);
	return 0;
}


// Source: object-name.c
// Lines 1210-1228
