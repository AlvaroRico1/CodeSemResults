static int open_loose_object(struct repository *r,
			     const struct object_id *oid, const char **path)
{
	int fd;
	struct object_directory *odb;
	int most_interesting_errno = ENOENT;
	static struct strbuf buf = STRBUF_INIT;

	prepare_alt_odb(r);
	for (odb = r->objects->odb; odb; odb = odb->next) {
		*path = odb_loose_path(odb, &buf, oid);
		fd = git_open(*path);
		if (fd >= 0)
			return fd;

		if (most_interesting_errno == ENOENT)
			most_interesting_errno = errno;
	}
	errno = most_interesting_errno;
	return -1;
}


// Source: object-file.c
// Lines 1115-1135
