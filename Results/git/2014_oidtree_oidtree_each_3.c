void oidtree_each(struct oidtree *ot, const struct object_id *oid,
			size_t oidhexsz, oidtree_iter fn, void *arg)
{
	size_t klen = oidhexsz / 2;
	struct oidtree_iter_data x = { 0 };
	assert(oidhexsz <= GIT_MAX_HEXSZ);

	x.fn = fn;
	x.arg = arg;
	x.algo = oid->algo;
	if (oidhexsz & 1) {
		x.last_byte = oid->hash[klen];
		x.last_nibble_at = &klen;
	}
	cb_each(&ot->tree, (const uint8_t *)oid, klen, iter, &x);
}


// Source: oidtree.c
// Lines 94-109
