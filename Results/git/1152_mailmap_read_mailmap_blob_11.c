static int read_mailmap_blob(struct string_list *map, const char *name)
{
	struct object_id oid;
	char *buf;
	unsigned long size;
	enum object_type type;

	if (!name)
		return 0;
	if (get_oid(name, &oid) < 0)
		return 0;

	buf = read_object_file(&oid, &type, &size);
	if (!buf)
		return error("unable to read mailmap object at %s", name);
	if (type != OBJ_BLOB)
		return error("mailmap is not a blob: %s", name);

	read_mailmap_string(map, buf);

	free(buf);
	return 0;
}


// Source: mailmap.c
// Lines 207-229
