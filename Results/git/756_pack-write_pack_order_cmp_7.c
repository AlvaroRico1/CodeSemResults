static int pack_order_cmp(const void *va, const void *vb, void *ctx)
{
	struct pack_idx_entry **objects = ctx;

	off_t oa = objects[*(uint32_t*)va]->offset;
	off_t ob = objects[*(uint32_t*)vb]->offset;

	if (oa < ob)
		return -1;
	if (oa > ob)
		return 1;
	return 0;
}


// Source: pack-write.c
// Lines 168-180
