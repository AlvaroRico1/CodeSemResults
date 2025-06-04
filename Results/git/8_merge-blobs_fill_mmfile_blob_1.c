static int fill_mmfile_blob(mmfile_t *f, struct blob *obj)
{
	void *buf;
	unsigned long size;
	enum object_type type;

	buf = read_object_file(&obj->object.oid, &type, &size);
	if (!buf)
		return -1;
	if (type != OBJ_BLOB) {
		free(buf);
		return -1;
	}
	f->ptr = buf;
	f->size = size;
	return 0;
}


// Source: merge-blobs.c
// Lines 9-25
