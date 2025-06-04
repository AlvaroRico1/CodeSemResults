static int queue_difference(const git_index_entry **entries, void *data)
{
	struct merge_diff_find_data *find_data = data;
	bool item_modified = false;
	size_t i;

	if (!entries[0] || !entries[1] || !entries[2]) {
		item_modified = true;
	} else {
		for (i = 1; i < 3; i++) {
			if (index_entry_cmp(entries[0], entries[i]) != 0) {
				item_modified = true;
				break;
			}
		}
	}

	return item_modified ?
		merge_diff_list_insert_conflict(
			find_data->diff_list, &find_data->df_data, entries) :
		merge_diff_list_insert_unmodified(find_data->diff_list, entries);
}


// Source: merge.c
// Lines 1799-1820
