int git_treebuilder_filter(
	git_treebuilder *bld,
	git_treebuilder_filter_cb filter,
	void *payload)
{
	const char *filename;
	git_tree_entry *entry;

	GIT_ASSERT_ARG(bld);
	GIT_ASSERT_ARG(filter);

	git_strmap_foreach(bld->map, filename, entry, {
			if (filter(entry, payload)) {
				git_strmap_delete(bld->map, filename);
				git_tree_entry_free(entry);
			}
	});

	return 0;
}


// Source: tree.c
// Lines 855-874
