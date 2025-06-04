int git_object_lookup_bypath(
		git_object **out,
		const git_object *treeish,
		const char *path,
		git_object_t type)
{
	int error = -1;
	git_tree *tree = NULL;
	git_tree_entry *entry = NULL;

	GIT_ASSERT_ARG(out);
	GIT_ASSERT_ARG(treeish);
	GIT_ASSERT_ARG(path);

	if ((error = git_object_peel((git_object**)&tree, treeish, GIT_OBJECT_TREE)) < 0 ||
		 (error = git_tree_entry_bypath(&entry, tree, path)) < 0)
	{
		goto cleanup;
	}

	if (type != GIT_OBJECT_ANY && git_tree_entry_type(entry) != type)
	{
		git_error_set(GIT_ERROR_OBJECT,
				"object at path '%s' is not of the asked-for type %d",
				path, type);
		error = GIT_EINVALIDSPEC;
		goto cleanup;
	}

	error = git_tree_entry_to_object(out, git_object_owner(treeish), entry);

cleanup:
	git_tree_entry_free(entry);
	git_tree_free(tree);
	return error;
}


// Source: object.c
// Lines 464-499
