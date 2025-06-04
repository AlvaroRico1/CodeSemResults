static int check_object_connectivity(git_indexer *idx, const git_rawobj *obj)
{
	git_object *object;
	git_oid *expected;
	int error = 0;

	if (obj->type != GIT_OBJECT_BLOB &&
	    obj->type != GIT_OBJECT_TREE &&
	    obj->type != GIT_OBJECT_COMMIT &&
	    obj->type != GIT_OBJECT_TAG)
		return 0;

	if (git_object__from_raw(&object, obj->data, obj->len, obj->type) < 0) {
		/*
		 * parse_raw returns EINVALID on invalid data; downgrade
		 * that to a normal -1 error code.
		 */
		error = -1;
		goto out;
	}

	if ((expected = git_oidmap_get(idx->expected_oids, &object->cached.oid)) != NULL) {
		git_oidmap_delete(idx->expected_oids, &object->cached.oid);
		git__free(expected);
	}

	/*
	 * Check whether this is a known object. If so, we can just continue as
	 * we assume that the ODB has a complete graph.
	 */
	if (idx->odb && git_odb_exists(idx->odb, &object->cached.oid))
		return 0;

	switch (obj->type) {
		case GIT_OBJECT_TREE:
		{
			git_tree *tree = (git_tree *) object;
			git_tree_entry *entry;
			size_t i;

			git_array_foreach(tree->entries, i, entry)
				if (add_expected_oid(idx, entry->oid) < 0)
					goto out;

			break;
		}


// Source: indexer.c
// Lines 347-392
