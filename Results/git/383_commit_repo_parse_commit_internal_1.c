int repo_parse_commit_internal(struct repository *r,
			       struct commit *item,
			       int quiet_on_missing,
			       int use_commit_graph)
{
	enum object_type type;
	void *buffer;
	unsigned long size;
	int ret;

	if (!item)
		return -1;
	if (item->object.parsed)
		return 0;
	if (use_commit_graph && parse_commit_in_graph(r, item))
		return 0;
	buffer = repo_read_object_file(r, &item->object.oid, &type, &size);
	if (!buffer)
		return quiet_on_missing ? -1 :
			error("Could not read %s",
			     oid_to_hex(&item->object.oid));
	if (type != OBJ_COMMIT) {
		free(buffer);
		return error("Object %s not a commit",
			     oid_to_hex(&item->object.oid));
	}

	ret = parse_commit_buffer(r, item, buffer, size, 0);
	if (save_commit_buffer && !ret) {
		set_commit_buffer(r, item, buffer, size);
		return 0;
	}
	free(buffer);
	return ret;
}


// Source: commit.c
// Lines 473-507
