static uint32_t *midx_pack_order(struct write_midx_context *ctx)
{
	struct midx_pack_order_data *data;
	uint32_t *pack_order;
	uint32_t i;

	ALLOC_ARRAY(data, ctx->entries_nr);
	for (i = 0; i < ctx->entries_nr; i++) {
		struct pack_midx_entry *e = &ctx->entries[i];
		data[i].nr = i;
		data[i].pack = ctx->pack_perm[e->pack_int_id];
		if (!e->preferred)
			data[i].pack |= (1U << 31);
		data[i].offset = e->offset;
	}

	QSORT(data, ctx->entries_nr, midx_pack_order_cmp);

	ALLOC_ARRAY(pack_order, ctx->entries_nr);
	for (i = 0; i < ctx->entries_nr; i++)
		pack_order[i] = data[i].nr;
	free(data);

	return pack_order;
}


// Source: midx.c
// Lines 834-858
