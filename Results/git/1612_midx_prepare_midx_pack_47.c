int prepare_midx_pack(struct repository *r, struct multi_pack_index *m, uint32_t pack_int_id)
{
	struct strbuf pack_name = STRBUF_INIT;
	struct packed_git *p;

	if (pack_int_id >= m->num_packs)
		die(_("bad pack-int-id: %u (%u total packs)"),
		    pack_int_id, m->num_packs);

	if (m->packs[pack_int_id])
		return 0;

	strbuf_addf(&pack_name, "%s/pack/%s", m->object_dir,
		    m->pack_names[pack_int_id]);

	p = add_packed_git(pack_name.buf, pack_name.len, m->local);
	strbuf_release(&pack_name);

	if (!p)
		return 1;

	p->multi_pack_index = 1;
	m->packs[pack_int_id] = p;
	install_packed_git(r, p);
	list_add_tail(&p->mru, &r->objects->packed_git_mru);

	return 0;
}


// Source: midx.c
// Lines 215-242
