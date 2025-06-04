static int git_treebuilder__write_with_buffer(
	git_oid *oid,
	git_treebuilder *bld,
	git_str *buf)
{
	int error = 0;
	size_t i, entrycount;
	git_odb *odb;
	git_tree_entry *entry;
	git_vector entries = GIT_VECTOR_INIT;

	git_str_clear(buf);

	entrycount = git_strmap_size(bld->map);
	if ((error = git_vector_init(&entries, entrycount, entry_sort_cmp)) < 0)
		goto out;

	if (buf->asize == 0 &&
	    (error = git_str_grow(buf, entrycount * 72)) < 0)
		goto out;

	git_strmap_foreach_value(bld->map, entry, {
		if ((error = git_vector_insert(&entries, entry)) < 0)
			goto out;
	});

	git_vector_sort(&entries);

	for (i = 0; i < entries.length && !error; ++i) {
		entry = git_vector_get(&entries, i);

		git_str_printf(buf, "%o ", entry->attr);
		git_str_put(buf, entry->filename, entry->filename_len + 1);
		git_str_put(buf, (char *)entry->oid->id, GIT_OID_RAWSZ);

		if (git_str_oom(buf)) {
			error = -1;
			goto out;
		}
	}

	if ((error = git_repository_odb__weakptr(&odb, bld->repo)) == 0)
		error = git_odb_write(oid, odb, buf->ptr, buf->size, GIT_OBJECT_TREE);

out:
	git_vector_free(&entries);

	return error;
}


// Source: tree.c
// Lines 506-554
