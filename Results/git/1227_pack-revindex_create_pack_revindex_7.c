static void create_pack_revindex(struct packed_git *p)
{
	const unsigned num_ent = p->num_objects;
	unsigned i;
	const char *index = p->index_data;
	const unsigned hashsz = the_hash_algo->rawsz;

	ALLOC_ARRAY(p->revindex, num_ent + 1);
	index += 4 * 256;

	if (p->index_version > 1) {
		const uint32_t *off_32 =
			(uint32_t *)(index + 8 + (size_t)p->num_objects * (hashsz + 4));
		const uint32_t *off_64 = off_32 + p->num_objects;
		for (i = 0; i < num_ent; i++) {
			const uint32_t off = ntohl(*off_32++);
			if (!(off & 0x80000000)) {
				p->revindex[i].offset = off;
			} else {
				p->revindex[i].offset = get_be64(off_64);
				off_64 += 2;
			}
			p->revindex[i].nr = i;
		}
	} else {
		for (i = 0; i < num_ent; i++) {
			const uint32_t hl = *((uint32_t *)(index + (hashsz + 4) * i));
			p->revindex[i].offset = ntohl(hl);
			p->revindex[i].nr = i;
		}
	}


// Source: pack-revindex.c
// Lines 128-158
