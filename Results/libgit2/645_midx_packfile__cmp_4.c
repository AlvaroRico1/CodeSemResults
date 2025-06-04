static int packfile__cmp(const void *a_, const void *b_)
{
	const struct git_pack_file *a = a_;
	const struct git_pack_file *b = b_;

	return strcmp(a->pack_name, b->pack_name);
}


// Source: midx.c
// Lines 492-498
