static void handle_directory_level_conflicts(struct merge_options *opt,
					     struct hashmap *dir_re_head,
					     struct tree *head,
					     struct hashmap *dir_re_merge,
					     struct tree *merge)
{
	struct hashmap_iter iter;
	struct dir_rename_entry *head_ent;
	struct dir_rename_entry *merge_ent;

	struct string_list remove_from_head = STRING_LIST_INIT_NODUP;
	struct string_list remove_from_merge = STRING_LIST_INIT_NODUP;

	hashmap_for_each_entry(dir_re_head, &iter, head_ent,
				ent /* member name */) {
		merge_ent = dir_rename_find_entry(dir_re_merge, head_ent->dir);
		if (merge_ent &&
		    !head_ent->non_unique_new_dir &&
		    !merge_ent->non_unique_new_dir &&
		    !strbuf_cmp(&head_ent->new_dir, &merge_ent->new_dir)) {
			/* 1. Renamed identically; remove it from both sides */
			string_list_append(&remove_from_head,
					   head_ent->dir)->util = head_ent;
			strbuf_release(&head_ent->new_dir);
			string_list_append(&remove_from_merge,
					   merge_ent->dir)->util = merge_ent;
			strbuf_release(&merge_ent->new_dir);
		} else if (tree_has_path(opt->repo, head, head_ent->dir)) {
			/* 2. This wasn't a directory rename after all */
			string_list_append(&remove_from_head,
					   head_ent->dir)->util = head_ent;
			strbuf_release(&head_ent->new_dir);
		}
	}

	remove_hashmap_entries(dir_re_head, &remove_from_head);
	remove_hashmap_entries(dir_re_merge, &remove_from_merge);

	hashmap_for_each_entry(dir_re_merge, &iter, merge_ent,
				ent /* member name */) {
		head_ent = dir_rename_find_entry(dir_re_head, merge_ent->dir);
		if (tree_has_path(opt->repo, merge, merge_ent->dir)) {
			/* 2. This wasn't a directory rename after all */
			string_list_append(&remove_from_merge,
					   merge_ent->dir)->util = merge_ent;
		} else if (head_ent &&
			   !head_ent->non_unique_new_dir &&
			   !merge_ent->non_unique_new_dir) {
			/* 3. rename/rename(1to2) */
			/*
			 * We can assume it's not rename/rename(1to1) because
			 * that was case (1), already checked above.  So we
			 * know that head_ent->new_dir and merge_ent->new_dir
			 * are different strings.
			 */
			output(opt, 1, _("CONFLICT (rename/rename): "
				       "Rename directory %s->%s in %s. "
				       "Rename directory %s->%s in %s"),
			       head_ent->dir, head_ent->new_dir.buf, opt->branch1,
			       head_ent->dir, merge_ent->new_dir.buf, opt->branch2);
			string_list_append(&remove_from_head,
					   head_ent->dir)->util = head_ent;
			strbuf_release(&head_ent->new_dir);
			string_list_append(&remove_from_merge,
					   merge_ent->dir)->util = merge_ent;
			strbuf_release(&merge_ent->new_dir);
		}
	}

	remove_hashmap_entries(dir_re_head, &remove_from_head);
	remove_hashmap_entries(dir_re_merge, &remove_from_merge);
}

static struct hashmap *get_directory_renames(struct diff_queue_struct *pairs)
{
	struct hashmap *dir_renames;
	struct hashmap_iter iter;
	struct dir_rename_entry *entry;
	int i;

	/*
	 * Typically, we think of a directory rename as all files from a
	 * certain directory being moved to a target directory.  However,
	 * what if someone first moved two files from the original
	 * directory in one commit, and then renamed the directory
	 * somewhere else in a later commit?  At merge time, we just know
	 * that files from the original directory went to two different
	 * places, and that the bulk of them ended up in the same place.
	 * We want each directory rename to represent where the bulk of the
	 * files from that directory end up; this function exists to find
	 * where the bulk of the files went.
	 *
	 * The first loop below simply iterates through the list of file
	 * renames, finding out how often each directory rename pair
	 * possibility occurs.
	 */
	dir_renames = xmalloc(sizeof(*dir_renames));
	dir_rename_init(dir_renames);
	for (i = 0; i < pairs->nr; ++i) {
		struct string_list_item *item;
		int *count;
		struct diff_filepair *pair = pairs->queue[i];
		char *old_dir, *new_dir;

		/* File not part of directory rename if it wasn't renamed */
		if (pair->status != 'R')
			continue;

		get_renamed_dir_portion(pair->one->path, pair->two->path,
					&old_dir,        &new_dir);
		if (!old_dir)
			/* Directory didn't change at all; ignore this one. */
			continue;

		entry = dir_rename_find_entry(dir_renames, old_dir);
		if (!entry) {
			entry = xmalloc(sizeof(*entry));
			dir_rename_entry_init(entry, old_dir);
			hashmap_put(dir_renames, &entry->ent);
		} else {
			free(old_dir);
		}
		item = string_list_lookup(&entry->possible_new_dirs, new_dir);
		if (!item) {
			item = string_list_insert(&entry->possible_new_dirs,
						  new_dir);
			item->util = xcalloc(1, sizeof(int));
		} else {
			free(new_dir);
		}
		count = item->util;
		*count += 1;
	}


// Source: merge-recursive.c
// Lines 2180-2312
