static int idx_or_pack_name_cmp(const void *_va, const void *_vb)
{
	const char *pack_name = _va;
	const struct pack_info *compar = _vb;

	return cmp_idx_or_pack_name(pack_name, compar->pack_name);
}


// Source: midx.c
// Lines 438-444
