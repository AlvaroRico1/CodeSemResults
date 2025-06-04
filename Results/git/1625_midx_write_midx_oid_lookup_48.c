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


// Source: midx.c
// Lines 729-752
