static void handle_directory_level_conflicts(struct merge_options *opt)
{
	struct hashmap_iter iter;
	struct strmap_entry *entry;
	struct string_list duplicated = STRING_LIST_INIT_NODUP;
	struct rename_info *renames = &opt->priv->renames;
	struct strmap *side1_dir_renames = &renames->dir_renames[MERGE_SIDE1];
	struct strmap *side2_dir_renames = &renames->dir_renames[MERGE_SIDE2];
	int i;

	strmap_for_each_entry(side1_dir_renames, &iter, entry) {
		if (strmap_contains(side2_dir_renames, entry->key))
			string_list_append(&duplicated, entry->key);
	}

	for (i = 0; i < duplicated.nr; i++) {
		strmap_remove(side1_dir_renames, duplicated.items[i].string, 0);
		strmap_remove(side2_dir_renames, duplicated.items[i].string, 0);
	}


// Source: merge-ort.c
// Lines 2130-2148
