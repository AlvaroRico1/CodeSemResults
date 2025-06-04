int add_patterns_from_blob_to_list(
	struct object_id *oid,
	const char *base, int baselen,
	struct pattern_list *pl)
{
	char *buf;
	size_t size;
	int r;

	r = do_read_blob(oid, NULL, &size, &buf);
	if (r != 1)
		return r;

	add_patterns_from_buffer(buf, size, base, baselen, pl);
	return 0;
}


// Source: dir.c
// Lines 1167-1182
