static int handle_colon_syntax(
	git_object **out,
	git_object *obj,
	const char *path)
{
	git_object *tree;
	int error = -1;
	git_tree_entry *entry = NULL;

	if ((error = git_object_peel(&tree, obj, GIT_OBJECT_TREE)) < 0)
		return error == GIT_ENOTFOUND ? GIT_EINVALIDSPEC : error;

	if (*path == '\0') {
		*out = tree;
		return 0;
	}

	/*
	 * TODO: Handle the relative path syntax
	 * (:./relative/path and :../relative/path)
	 */
	if ((error = git_tree_entry_bypath(&entry, (git_tree *)tree, path)) < 0)
		goto cleanup;

	error = git_tree_entry_to_object(out, git_object_owner(tree), entry);

cleanup:
	git_tree_entry_free(entry);
	git_object_free(tree);

	return error;
}


// Source: revparse.c
// Lines 427-458
