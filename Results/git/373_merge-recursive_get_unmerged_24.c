static struct string_list *get_unmerged(struct index_state *istate)
{
	struct string_list *unmerged = xcalloc(1, sizeof(struct string_list));
	int i;

	unmerged->strdup_strings = 1;

	/* TODO: audit for interaction with sparse-index. */
	ensure_full_index(istate);
	for (i = 0; i < istate->cache_nr; i++) {
		struct string_list_item *item;
		struct stage_data *e;
		const struct cache_entry *ce = istate->cache[i];
		if (!ce_stage(ce))
			continue;

		item = string_list_lookup(unmerged, ce->name);
		if (!item) {
			item = string_list_insert(unmerged, ce->name);
			item->util = xcalloc(1, sizeof(struct stage_data));
		}


// Source: merge-recursive.c
// Lines 515-535
