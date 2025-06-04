static int pack_objects_insert_tree(git_packbuilder *pb, git_tree *tree)
{
	size_t i;
	int error;
	git_tree *subtree;
	struct walk_object *obj;
	const char *name;

	if ((error = retrieve_object(&obj, pb, git_tree_id(tree))) < 0)
		return error;

	if (obj->seen || obj->uninteresting)
		return 0;

	obj->seen = 1;

	if ((error = git_packbuilder_insert(pb, &obj->id, NULL)))
		return error;

	for (i = 0; i < git_tree_entrycount(tree); i++) {
		const git_tree_entry *entry = git_tree_entry_byindex(tree, i);
		const git_oid *entry_id = git_tree_entry_id(entry);
		switch (git_tree_entry_type(entry)) {
		case GIT_OBJECT_TREE:
			if ((error = git_tree_lookup(&subtree, pb->repo, entry_id)) < 0)
				return error;

			error = pack_objects_insert_tree(pb, subtree);
			git_tree_free(subtree);

			if (error < 0)
				return error;

			break;
		case GIT_OBJECT_BLOB:
			if ((error = retrieve_object(&obj, pb, entry_id)) < 0)
				return error;
			if (obj->uninteresting)
				continue;
			name = git_tree_entry_name(entry);
			if ((error = git_packbuilder_insert(pb, entry_id, name)) < 0)
				return error;
			break;
		default:
			/* it's a submodule or something unknown, we don't want it */
			;
		}
	}


	return error;
}


// Source: pack-objects.c
// Lines 1663-1714
