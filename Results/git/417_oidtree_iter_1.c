static enum cb_next iter(struct cb_node *n, void *arg)
{
	struct oidtree_iter_data *x = arg;
	struct object_id k;

	/* Copy to provide 4-byte alignment needed by struct object_id. */
	memcpy(&k, n->k, sizeof(k));

	if (x->algo != GIT_HASH_UNKNOWN && x->algo != k.algo)
		return CB_CONTINUE;

	if (x->last_nibble_at) {
		if ((k.hash[*x->last_nibble_at] ^ x->last_byte) & 0xf0)
			return CB_CONTINUE;
	}

	return x->fn(&k, x->arg);
}


// Source: oidtree.c
// Lines 75-92
