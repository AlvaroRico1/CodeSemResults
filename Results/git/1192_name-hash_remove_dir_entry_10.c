static void remove_dir_entry(struct index_state *istate, struct cache_entry *ce)
{
	/*
	 * Release reference to the directory entry. If 0, remove and continue
	 * with parent directory.
	 */
	struct dir_entry *dir = hash_dir_entry(istate, ce, ce_namelen(ce));
	while (dir && !(--dir->nr)) {
		struct dir_entry *parent = dir->parent;
		hashmap_remove(&istate->dir_hash, &dir->ent, NULL);
		free(dir);
		dir = parent;
	}


// Source: name-hash.c
// Lines 93-105
