static char *create_temp_file(struct object_id *oid)
{
	static char path[50];
	void *buf;
	enum object_type type;
	unsigned long size;
	int fd;

	buf = read_object_file(oid, &type, &size);
	if (!buf || type != OBJ_BLOB)
		die("unable to read blob object %s", oid_to_hex(oid));

	xsnprintf(path, sizeof(path), ".merge_file_XXXXXX");
	fd = xmkstemp(path);
	if (write_in_full(fd, buf, size) < 0)
		die_errno("unable to write temp-file");
	close(fd);
	return path;
}


// Source: unpack-file.c
// Lines 5-23
