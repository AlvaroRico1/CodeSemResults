static void *get_delta(struct object_entry *entry)
{
	unsigned long size, base_size, delta_size;
	void *buf, *base_buf, *delta_buf;
	enum object_type type;

	buf = read_object_file(&entry->idx.oid, &type, &size);
	if (!buf)
		die(_("unable to read %s"), oid_to_hex(&entry->idx.oid));
	base_buf = read_object_file(&DELTA(entry)->idx.oid, &type,
				    &base_size);
	if (!base_buf)
		die("unable to read %s",
		    oid_to_hex(&DELTA(entry)->idx.oid));
	delta_buf = diff_delta(base_buf, base_size,
			       buf, size, &delta_size, 0);
	/*
	 * We successfully computed this delta once but dropped it for
	 * memory reasons. Something is very wrong if this time we
	 * recompute and create a different delta.
	 */
	if (!delta_buf || delta_size != DELTA_SIZE(entry))
		BUG("delta size changed");
	free(buf);
	free(base_buf);
	return delta_buf;
}


// Source: pack-objects.c
// Lines 284-310
