static git_odb_object *odb_object__alloc(const git_oid *oid, git_rawobj *source)
{
	git_odb_object *object = git__calloc(1, sizeof(git_odb_object));

	if (object != NULL) {
		git_oid_cpy(&object->cached.oid, oid);
		object->cached.type = source->type;
		object->cached.size = source->len;
		object->buffer      = source->data;
	}

	return object;
}


// Source: odb.c
// Lines 141-153
