static void add_dir_entry(struct index_state *istate, struct cache_entry *ce)
{
	/* Add reference to the directory entry (and parents if 0). */
	struct dir_entry *dir = hash_dir_entry(istate, ce, ce_namelen(ce));
	while (dir && !(dir->nr++))
		dir = dir->parent;
}


// Source: name-hash.c
// Lines 85-91
