static int stat_loose_object(struct repository *r, const struct object_id *oid,
			     struct stat *st, const char **path)
{
	struct object_directory *odb;
	static struct strbuf buf = STRBUF_INIT;

	prepare_alt_odb(r);
	for (odb = r->objects->odb; odb; odb = odb->next) {
		*path = odb_loose_path(odb, &buf, oid);
		if (!lstat(*path, st))
			return 0;
	}

	return -1;
}


// Source: object-file.c
// Lines 1095-1109
