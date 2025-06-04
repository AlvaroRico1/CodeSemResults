static int write_midx_large_offsets(struct hashfile *f,
				    void *data)
{
	struct write_midx_context *ctx = data;
	struct pack_midx_entry *list = ctx->entries;
	struct pack_midx_entry *end = ctx->entries + ctx->entries_nr;
	uint32_t nr_large_offset = ctx->num_large_offsets;

	while (nr_large_offset) {
		struct pack_midx_entry *obj;
		uint64_t offset;

		if (list >= end)
			BUG("too many large-offset objects");

		obj = list++;
		offset = obj->offset;

		if (!(offset >> 31))
			continue;

		hashwrite_be64(f, offset);

		nr_large_offset--;
	}

	return 0;
}


// Source: midx.c
// Lines 784-811
