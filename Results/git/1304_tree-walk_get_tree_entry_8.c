int get_tree_entry(struct repository *r,
		   const struct object_id *tree_oid,
		   const char *name,
		   struct object_id *oid,
		   unsigned short *mode)
{
	int retval;
	void *tree;
	unsigned long size;
	struct object_id root;

	tree = read_object_with_reference(r, tree_oid, tree_type, &size, &root);
	if (!tree)
		return -1;

	if (name[0] == '\0') {
		oidcpy(oid, &root);
		free(tree);
		return 0;
	}

	if (!size) {
		retval = -1;
	} else {
		struct tree_desc t;
		init_tree_desc(&t, tree, size);
		retval = find_tree_entry(r, &t, name, oid, mode);
	}
	free(tree);
	return retval;
}


// Source: tree-walk.c
// Lines 597-627
