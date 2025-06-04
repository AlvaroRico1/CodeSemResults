static int midx_read_oid_fanout(const unsigned char *chunk_start,
				size_t chunk_size, void *data)
{
	struct multi_pack_index *m = data;
	m->chunk_oid_fanout = (uint32_t *)chunk_start;

	if (chunk_size != 4 * 256) {
		error(_("multi-pack-index OID fanout is of the wrong size"));
		return 1;
	}
	return 0;
}


// Source: midx.c
// Lines 71-82
