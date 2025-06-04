int nfs_map_string_to_numeric(const char *name, size_t namelen, __u32 *res)
{
	unsigned long val;
	char buf[16];

	if (memchr(name, '@', namelen) != NULL || namelen >= sizeof(buf))
		return 0;
	memcpy(buf, name, namelen);
	buf[namelen] = '\0';
	if (kstrtoul(buf, 0, &val) != 0)
		return 0;
	*res = val;
	return 1;
}


// Source: nfs4idmap.c
// Lines 164-177
