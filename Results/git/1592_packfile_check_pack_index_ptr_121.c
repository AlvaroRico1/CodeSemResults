void check_pack_index_ptr(const struct packed_git *p, const void *vptr)
{
	const unsigned char *ptr = vptr;
	const unsigned char *start = p->index_data;
	const unsigned char *end = start + p->index_size;
	if (ptr < start)
		die(_("offset before start of pack index for %s (corrupt index?)"),
		    p->pack_name);
	/* No need to check for underflow; .idx files must be at least 8 bytes */
	if (ptr >= end - 8)
		die(_("offset beyond end of pack index for %s (truncated index?)"),
		    p->pack_name);
}


// Source: packfile.c
// Lines 1917-1929
