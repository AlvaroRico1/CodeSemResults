static void *lazy_name_thread_proc(void *_data)
{
	struct lazy_name_thread_data *d = _data;
	int k;

	for (k = 0; k < d->istate->cache_nr; k++) {
		struct cache_entry *ce_k = d->istate->cache[k];
		ce_k->ce_flags |= CE_HASHED;
		hashmap_entry_init(&ce_k->ent, d->lazy_entries[k].hash_name);
		hashmap_add(&d->istate->name_hash, &ce_k->ent);
	}

	return NULL;
}


// Source: name-hash.c
// Lines 477-490
