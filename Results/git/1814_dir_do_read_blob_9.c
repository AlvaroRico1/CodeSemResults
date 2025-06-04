static int do_read_blob(const struct object_id *oid, struct oid_stat *oid_stat,
			size_t *size_out, char **data_out)
{
	enum object_type type;
	unsigned long sz;
	char *data;

	*size_out = 0;
	*data_out = NULL;

	data = read_object_file(oid, &type, &sz);
	if (!data || type != OBJ_BLOB) {
		free(data);
		return -1;
	}

	if (oid_stat) {
		memset(&oid_stat->stat, 0, sizeof(oid_stat->stat));
		oidcpy(&oid_stat->oid, oid);
	}

	if (sz == 0) {
		free(data);
		return 0;
	}

	if (data[sz - 1] != '\n') {
		data = xrealloc(data, st_add(sz, 1));
		data[sz++] = '\n';
	}

	*size_out = xsize_t(sz);
	*data_out = data;

	return 1;
}


// Source: dir.c
// Lines 260-295
