int fill_midx_entry(struct repository * r,
		    const struct object_id *oid,
		    struct pack_entry *e,
		    struct multi_pack_index *m)
{
	uint32_t pos;
	uint32_t pack_int_id;
	struct packed_git *p;

	if (!bsearch_midx(oid, m, &pos))
		return 0;

	if (pos >= m->num_objects)
		return 0;

	pack_int_id = nth_midxed_pack_int_id(m, pos);

	if (prepare_midx_pack(r, m, pack_int_id))
		return 0;
	p = m->packs[pack_int_id];

	/*
	* We are about to tell the caller where they can locate the
	* requested object.  We better make sure the packfile is
	* still here and can be accessed before supplying that
	* answer, as it may have been deleted since the MIDX was
	* loaded!
	*/
	if (!is_pack_valid(p))
		return 0;

	if (oidset_size(&p->bad_objects) &&
	    oidset_contains(&p->bad_objects, oid))
		return 0;

	e->offset = nth_midxed_offset(m, pos);
	e->p = p;

	return 1;
}


// Source: midx.c
// Lines 286-325
