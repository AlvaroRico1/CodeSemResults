int git_sortedcache_lookup_index(
	size_t *out, git_sortedcache *sc, const char *key)
{
	struct sortedcache_magic_key magic;

	magic.offset = sc->item_path_offset;
	magic.key    = key;

	return git_vector_bsearch2(out, &sc->items, sortedcache_magic_cmp, &magic);
}


// Source: sortedcache.c
// Lines 345-354
