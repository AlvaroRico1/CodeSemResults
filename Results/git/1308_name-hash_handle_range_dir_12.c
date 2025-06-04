static int handle_range_dir(
	struct index_state *istate,
	int k_start,
	int k_end,
	struct dir_entry *parent,
	struct strbuf *prefix,
	struct lazy_entry *lazy_entries,
	struct dir_entry **dir_new_out)
{
	int rc, k;
	int input_prefix_len = prefix->len;
	struct dir_entry *dir_new;

	dir_new = hash_dir_entry_with_parent_and_prefix(istate, parent, prefix);

	strbuf_addch(prefix, '/');

	/*
	 * Scan forward in the index array for index entries having the same
	 * path prefix (that are also in this directory).
	 */
	if (k_start + 1 >= k_end)
		k = k_end;
	else if (strncmp(istate->cache[k_start + 1]->name, prefix->buf, prefix->len) > 0)
		k = k_start + 1;
	else if (strncmp(istate->cache[k_end - 1]->name, prefix->buf, prefix->len) == 0)
		k = k_end;
	else {
		int begin = k_start;
		int end = k_end;
		assert(begin >= 0);
		while (begin < end) {
			int mid = begin + ((end - begin) >> 1);
			int cmp = strncmp(istate->cache[mid]->name, prefix->buf, prefix->len);
			if (cmp == 0) /* mid has same prefix; look in second part */
				begin = mid + 1;
			else if (cmp > 0) /* mid is past group; look in first part */
				end = mid;
			else
				die("cache entry out of order");
		}
		k = begin;
	}

	/*
	 * Recurse and process what we can of this subset [k_start, k).
	 */
	rc = handle_range_1(istate, k_start, k, dir_new, prefix, lazy_entries);

	strbuf_setlen(prefix, input_prefix_len);

	*dir_new_out = dir_new;
	return rc;
}


// Source: name-hash.c
// Lines 328-381
