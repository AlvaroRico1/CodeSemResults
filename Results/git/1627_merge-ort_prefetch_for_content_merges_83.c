static void prefetch_for_content_merges(struct merge_options *opt,
					struct string_list *plist)
{
	struct string_list_item *e;
	struct oid_array to_fetch = OID_ARRAY_INIT;

	if (opt->repo != the_repository || !has_promisor_remote())
		return;

	for (e = &plist->items[plist->nr-1]; e >= plist->items; --e) {
		/* char *path = e->string; */
		struct conflict_info *ci = e->util;
		int i;

		/* Ignore clean entries */
		if (ci->merged.clean)
			continue;

		/* Ignore entries that don't need a content merge */
		if (ci->match_mask || ci->filemask < 6 ||
		    !S_ISREG(ci->stages[1].mode) ||
		    !S_ISREG(ci->stages[2].mode) ||
		    oideq(&ci->stages[1].oid, &ci->stages[2].oid))
			continue;

		/* Also don't need content merge if base matches either side */
		if (ci->filemask == 7 &&
		    S_ISREG(ci->stages[0].mode) &&
		    (oideq(&ci->stages[0].oid, &ci->stages[1].oid) ||
		     oideq(&ci->stages[0].oid, &ci->stages[2].oid)))
			continue;

		for (i = 0; i < 3; i++) {
			unsigned side_mask = (1 << i);
			struct version_info *vi = &ci->stages[i];

			if ((ci->filemask & side_mask) &&
			    S_ISREG(vi->mode) &&
			    oid_object_info_extended(opt->repo, &vi->oid, NULL,
						     OBJECT_INFO_FOR_PREFETCH))
				oid_array_append(&to_fetch, &vi->oid);
		}
	}

	promisor_remote_get_direct(opt->repo, to_fetch.oid, to_fetch.nr);
	oid_array_clear(&to_fetch);
}

static void process_entries(struct merge_options *opt,
			    struct object_id *result_oid)
{
	struct hashmap_iter iter;
	struct strmap_entry *e;
	struct string_list plist = STRING_LIST_INIT_NODUP;
	struct string_list_item *entry;
	struct directory_versions dir_metadata = { STRING_LIST_INIT_NODUP,
						   STRING_LIST_INIT_NODUP,
						   NULL, 0 };

	trace2_region_enter("merge", "process_entries setup", opt->repo);
	if (strmap_empty(&opt->priv->paths)) {
		oidcpy(result_oid, opt->repo->hash_algo->empty_tree);
		return;
	}

	/* Hack to pre-allocate plist to the desired size */
	trace2_region_enter("merge", "plist grow", opt->repo);
	ALLOC_GROW(plist.items, strmap_get_size(&opt->priv->paths), plist.alloc);
	trace2_region_leave("merge", "plist grow", opt->repo);

	/* Put every entry from paths into plist, then sort */
	trace2_region_enter("merge", "plist copy", opt->repo);
	strmap_for_each_entry(&opt->priv->paths, &iter, e) {
		string_list_append(&plist, e->key)->util = e->value;
	}
	trace2_region_leave("merge", "plist copy", opt->repo);

	trace2_region_enter("merge", "plist special sort", opt->repo);
	plist.cmp = sort_dirs_next_to_their_children;
	string_list_sort(&plist);
	trace2_region_leave("merge", "plist special sort", opt->repo);

	trace2_region_leave("merge", "process_entries setup", opt->repo);

	/*
	 * Iterate over the items in reverse order, so we can handle paths
	 * below a directory before needing to handle the directory itself.
	 *
	 * This allows us to write subtrees before we need to write trees,
	 * and it also enables sane handling of directory/file conflicts
	 * (because it allows us to know whether the directory is still in
	 * the way when it is time to process the file at the same path).
	 */
	trace2_region_enter("merge", "processing", opt->repo);
	prefetch_for_content_merges(opt, &plist);
	for (entry = &plist.items[plist.nr-1]; entry >= plist.items; --entry) {
		char *path = entry->string;
		/*
		 * NOTE: mi may actually be a pointer to a conflict_info, but
		 * we have to check mi->clean first to see if it's safe to
		 * reassign to such a pointer type.
		 */
		struct merged_info *mi = entry->util;

		write_completed_directory(opt, mi->directory_name,
					  &dir_metadata);
		if (mi->clean)
			record_entry_for_tree(&dir_metadata, path, mi);
		else {
			struct conflict_info *ci = (struct conflict_info *)mi;
			process_entry(opt, path, ci, &dir_metadata);
		}
	}


// Source: merge-ort.c
// Lines 3899-4011
