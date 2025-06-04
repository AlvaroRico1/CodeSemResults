static int write_midx_oid_lookup(struct hashfile *f,
				 void *data)
{
	struct write_midx_context *ctx = data;
	unsigned char hash_len = the_hash_algo->rawsz;
	struct pack_midx_entry *list = ctx->entries;
	uint32_t i;

	for (i = 0; i < ctx->entries_nr; i++) {
		struct pack_midx_entry *obj = list++;

		if (i < ctx->entries_nr - 1) {
			struct pack_midx_entry *next = list;
			if (oidcmp(&obj->oid, &next->oid) >= 0)
				BUG("OIDs not in order: %s >= %s",
				    oid_to_hex(&obj->oid),
				    oid_to_hex(&next->oid));
		}

		hashwrite(f, obj->oid.hash, (int)hash_len);
	}

	return 0;
}

static int write_midx_object_offsets(struct hashfile *f,
				     void *data)
{
	struct write_midx_context *ctx = data;
	struct pack_midx_entry *list = ctx->entries;
	uint32_t i, nr_large_offset = 0;

	for (i = 0; i < ctx->entries_nr; i++) {
		struct pack_midx_entry *obj = list++;

		if (ctx->pack_perm[obj->pack_int_id] == PACK_EXPIRED)
			BUG("object %s is in an expired pack with int-id %d",
			    oid_to_hex(&obj->oid),
			    obj->pack_int_id);

		hashwrite_be32(f, ctx->pack_perm[obj->pack_int_id]);

		if (ctx->large_offsets_needed && obj->offset >> 31)
			hashwrite_be32(f, MIDX_LARGE_OFFSET_NEEDED | nr_large_offset++);
		else if (!ctx->large_offsets_needed && obj->offset >> 32)
			BUG("object %s requires a large offset (%"PRIx64") but the MIDX is not writing large offsets!",
			    oid_to_hex(&obj->oid),
			    obj->offset);
		else
			hashwrite_be32(f, (uint32_t)obj->offset);
	}

	return 0;
}


// Source: midx.c
// Lines 729-782
