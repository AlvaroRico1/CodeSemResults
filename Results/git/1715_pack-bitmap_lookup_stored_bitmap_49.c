static struct ewah_bitmap *lookup_stored_bitmap(struct stored_bitmap *st)
{
	struct ewah_bitmap *parent;
	struct ewah_bitmap *composed;

	if (st->xor == NULL)
		return st->root;

	composed = ewah_pool_new();
	parent = lookup_stored_bitmap(st->xor);
	ewah_xor(st->root, parent, composed);

	ewah_pool_free(st->root);
	st->root = composed;
	st->xor = NULL;

	return composed;
}


// Source: pack-bitmap.c
// Lines 109-126
