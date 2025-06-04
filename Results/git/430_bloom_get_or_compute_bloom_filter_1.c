struct bloom_filter *get_or_compute_bloom_filter(struct repository *r,
						 struct commit *c,
						 int compute_if_not_present,
						 const struct bloom_filter_settings *settings,
						 enum bloom_filter_computed *computed)
{
	struct bloom_filter *filter;
	int i;
	struct diff_options diffopt;

	if (computed)
		*computed = BLOOM_NOT_COMPUTED;

	if (!bloom_filters.slab_size)
		return NULL;

	filter = bloom_filter_slab_at(&bloom_filters, c);

	if (!filter->data) {
		load_commit_graph_info(r, c);
		if (commit_graph_position(c) != COMMIT_NOT_FROM_GRAPH)
			load_bloom_filter_from_graph(r->objects->commit_graph, filter, c);
	}

	if (filter->data && filter->len)
		return filter;
	if (!compute_if_not_present)
		return NULL;

	repo_diff_setup(r, &diffopt);
	diffopt.flags.recursive = 1;
	diffopt.detect_rename = 0;
	diffopt.max_changes = settings->max_changed_paths;
	diff_setup_done(&diffopt);

	/* ensure commit is parsed so we have parent information */
	repo_parse_commit(r, c);

	if (c->parents)
		diff_tree_oid(&c->parents->item->object.oid, &c->object.oid, "", &diffopt);
	else
		diff_tree_oid(NULL, &c->object.oid, "", &diffopt);
	diffcore_std(&diffopt);

	if (diff_queued_diff.nr <= settings->max_changed_paths) {
		struct hashmap pathmap = HASHMAP_INIT(pathmap_cmp, NULL);
		struct pathmap_hash_entry *e;
		struct hashmap_iter iter;

		for (i = 0; i < diff_queued_diff.nr; i++) {
			const char *path = diff_queued_diff.queue[i]->two->path;

			/*
			 * Add each leading directory of the changed file, i.e. for
			 * 'dir/subdir/file' add 'dir' and 'dir/subdir' as well, so
			 * the Bloom filter could be used to speed up commands like
			 * 'git log dir/subdir', too.
			 *
			 * Note that directories are added without the trailing '/'.
			 */
			do {
				char *last_slash = strrchr(path, '/');

				FLEX_ALLOC_STR(e, path, path);
				hashmap_entry_init(&e->entry, strhash(path));

				if (!hashmap_get(&pathmap, &e->entry, NULL))
					hashmap_add(&pathmap, &e->entry);
				else
					free(e);

				if (!last_slash)
					last_slash = (char*)path;
				*last_slash = '\0';

			} while (*path);

			diff_free_filepair(diff_queued_diff.queue[i]);
		}


// Source: bloom.c
// Lines 187-265
