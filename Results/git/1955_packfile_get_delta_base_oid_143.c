static int get_delta_base_oid(struct packed_git *p,
			      struct pack_window **w_curs,
			      off_t curpos,
			      struct object_id *oid,
			      enum object_type type,
			      off_t delta_obj_offset)
{
	if (type == OBJ_REF_DELTA) {
		unsigned char *base = use_pack(p, w_curs, curpos, NULL);
		oidread(oid, base);
		return 0;
	} else if (type == OBJ_OFS_DELTA) {
		uint32_t base_pos;
		off_t base_offset = get_delta_base(p, w_curs, &curpos,
						   type, delta_obj_offset);

		if (!base_offset)
			return -1;

		if (offset_to_pack_pos(p, base_offset, &base_pos) < 0)
			return -1;

		return nth_packed_object_id(oid, p,
					    pack_pos_to_index(p, base_pos));
	} else


// Source: packfile.c
// Lines 1226-1250
