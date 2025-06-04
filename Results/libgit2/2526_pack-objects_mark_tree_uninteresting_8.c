static int mark_tree_uninteresting(git_packbuilder *pb, const git_oid *id)
{
	struct walk_object *obj;
	git_tree *tree;
	int error;
	size_t i;

	if ((error = retrieve_object(&obj, pb, id)) < 0)
		return error;

	if (obj->uninteresting)
		return 0;

	obj->uninteresting = 1;

	if ((error = git_tree_lookup(&tree, pb->repo, id)) < 0)
		return error;

	for (i = 0; i < git_tree_entrycount(tree); i++) {
		const git_tree_entry *entry = git_tree_entry_byindex(tree, i);
		const git_oid *entry_id = git_tree_entry_id(entry);
		switch (git_tree_entry_type(entry)) {
		case GIT_OBJECT_TREE:
			if ((error = mark_tree_uninteresting(pb, entry_id)) < 0)
				goto cleanup;
			break;
		case GIT_OBJECT_BLOB:
			if ((error = mark_blob_uninteresting(pb, entry_id)) < 0)
				goto cleanup;
			break;
		default:
			/* it's a submodule or something unknown, we don't want it */
			;
		}
	}

cleanup:
	git_tree_free(tree);
	return error;
}


// Source: pack-objects.c
// Lines 1594-1633
