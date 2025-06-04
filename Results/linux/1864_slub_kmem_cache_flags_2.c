slab_flags_t kmem_cache_flags(unsigned int object_size,
	slab_flags_t flags, const char *name,
	void (*ctor)(void *))
{
	char *iter;
	size_t len;

	/* If slub_debug = 0, it folds into the if conditional. */
	if (!slub_debug_slabs)
		return flags | slub_debug;

	len = strlen(name);
	iter = slub_debug_slabs;
	while (*iter) {
		char *end, *glob;
		size_t cmplen;

		end = strchrnul(iter, ',');

		glob = strnchr(iter, end - iter, '*');
		if (glob)
			cmplen = glob - iter;
		else
			cmplen = max_t(size_t, len, (end - iter));

		if (!strncmp(name, iter, cmplen)) {
			flags |= slub_debug;
			break;
		}

		if (!*end)
			break;
		iter = end + 1;
	}


// Source: slub.c
// Lines 1303-1336
