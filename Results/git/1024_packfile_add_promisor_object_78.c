static int add_promisor_object(const struct object_id *oid,
			       struct packed_git *pack,
			       uint32_t pos,
			       void *set_)
{
	struct oidset *set = set_;
	struct object *obj = parse_object(the_repository, oid);
	if (!obj)
		return 1;

	oidset_insert(set, oid);

	/*
	 * If this is a tree, commit, or tag, the objects it refers
	 * to are also promisor objects. (Blobs refer to no objects->)
	 */
	if (obj->type == OBJ_TREE) {
		struct tree *tree = (struct tree *)obj;
		struct tree_desc desc;
		struct name_entry entry;
		if (init_tree_desc_gently(&desc, tree->buffer, tree->size))
			/*
			 * Error messages are given when packs are
			 * verified, so do not print any here.
			 */
			return 0;
		while (tree_entry_gently(&desc, &entry))
			oidset_insert(set, &entry.oid);
		free_tree_buffer(tree);
	} else if (obj->type == OBJ_COMMIT) {
		struct commit *commit = (struct commit *) obj;
		struct commit_list *parents = commit->parents;

		oidset_insert(set, get_commit_tree_oid(commit));
		for (; parents; parents = parents->next)
			oidset_insert(set, &parents->item->object.oid);
	} else if (obj->type == OBJ_TAG) {
		struct tag *tag = (struct tag *) obj;
		oidset_insert(set, get_tagged_oid(tag));
	}
	return 0;
}


// Source: packfile.c
// Lines 2210-2251
