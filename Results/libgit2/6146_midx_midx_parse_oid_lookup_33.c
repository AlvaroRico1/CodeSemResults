static int midx_parse_oid_lookup(
		git_midx_file *idx,
		const unsigned char *data,
		struct git_midx_chunk *chunk_oid_lookup)
{
	uint32_t i;
	git_oid *oid, *prev_oid, zero_oid = {{0}};

	if (chunk_oid_lookup->offset == 0)
		return midx_error("missing OID Lookup chunk");
	if (chunk_oid_lookup->length == 0)
		return midx_error("empty OID Lookup chunk");
	if (chunk_oid_lookup->length != idx->num_objects * GIT_OID_RAWSZ)
		return midx_error("OID Lookup chunk has wrong length");

	idx->oid_lookup = oid = (git_oid *)(data + chunk_oid_lookup->offset);
	prev_oid = &zero_oid;
	for (i = 0; i < idx->num_objects; ++i, ++oid) {
		if (git_oid_cmp(prev_oid, oid) >= 0)
			return midx_error("OID Lookup index is non-monotonic");
		prev_oid = oid;
	}

	return 0;
}


// Source: midx.c
// Lines 112-136
