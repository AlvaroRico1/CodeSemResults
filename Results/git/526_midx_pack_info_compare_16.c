static int pack_info_compare(const void *_a, const void *_b)
{
	struct pack_info *a = (struct pack_info *)_a;
	struct pack_info *b = (struct pack_info *)_b;
	return strcmp(a->pack_name, b->pack_name);
}


// Source: midx.c
// Lines 431-436
