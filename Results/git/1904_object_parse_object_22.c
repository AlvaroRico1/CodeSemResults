struct object *parse_object(struct repository *r, const struct object_id *oid)
{
	unsigned long size;
	enum object_type type;
	int eaten;
	const struct object_id *repl = lookup_replace_object(r, oid);
	void *buffer;
	struct object *obj;

	obj = lookup_object(r, oid);
	if (obj && obj->parsed)
		return obj;

	if ((obj && obj->type == OBJ_BLOB && repo_has_object_file(r, oid)) ||
	    (!obj && repo_has_object_file(r, oid) &&
	     oid_object_info(r, oid, NULL) == OBJ_BLOB)) {
		if (check_object_signature(r, repl, NULL, 0, NULL) < 0) {
			error(_("hash mismatch %s"), oid_to_hex(oid));
			return NULL;
		}
		parse_blob_buffer(lookup_blob(r, oid), NULL, 0);
		return lookup_object(r, oid);
	}

	buffer = repo_read_object_file(r, oid, &type, &size);
	if (buffer) {
		if (check_object_signature(r, repl, buffer, size,
					   type_name(type)) < 0) {
			free(buffer);
			error(_("hash mismatch %s"), oid_to_hex(repl));
			return NULL;
		}

		obj = parse_object_buffer(r, oid, type, size,
					  buffer, &eaten);
		if (!eaten)
			free(buffer);
		return obj;
	}
	return NULL;
}


// Source: object.c
// Lines 266-306
