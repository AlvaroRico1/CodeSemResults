int parse_tag(struct tag *item)
{
	enum object_type type;
	void *data;
	unsigned long size;
	int ret;

	if (item->object.parsed)
		return 0;
	data = read_object_file(&item->object.oid, &type, &size);
	if (!data)
		return error("Could not read %s",
			     oid_to_hex(&item->object.oid));
	if (type != OBJ_TAG) {
		free(data);
		return error("Object %s not a tag",
			     oid_to_hex(&item->object.oid));
	}
	ret = parse_tag_buffer(the_repository, item, data, size);
	free(data);
	return ret;
}


// Source: tag.c
// Lines 209-230
