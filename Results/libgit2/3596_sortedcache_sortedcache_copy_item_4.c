static int sortedcache_copy_item(void *payload, void *tgt_item, void *src_item)
{
	git_sortedcache *sc = payload;
	/* path will already have been copied by upsert */
	memcpy(tgt_item, src_item, sc->item_path_offset);
	return 0;
}


// Source: sortedcache.c
// Lines 107-113
