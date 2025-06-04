const char *pack_basename(struct packed_git *p)
{
	const char *ret = strrchr(p->pack_name, '/');
	if (ret)
		ret = ret + 1; /* skip past slash */
	else
		ret = p->pack_name; /* we only have a base */
	return ret;
}


// Source: packfile.c
// Lines 505-513
