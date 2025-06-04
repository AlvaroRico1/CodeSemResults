int fsck_finish(struct fsck_options *options)
{
	int ret = 0;
	struct oidset_iter iter;
	const struct object_id *oid;

	oidset_iter_init(&options->gitmodules_found, &iter);
	while ((oid = oidset_iter_next(&iter))) {
		enum object_type type;
		unsigned long size;
		char *buf;

		if (oidset_contains(&options->gitmodules_done, oid))
			continue;

		buf = read_object_file(oid, &type, &size);
		if (!buf) {
			if (is_promisor_object(oid))
				continue;
			ret |= report(options,
				      oid, OBJ_BLOB,
				      FSCK_MSG_GITMODULES_MISSING,
				      "unable to read .gitmodules blob");
			continue;
		}

		if (type == OBJ_BLOB)
			ret |= fsck_blob(oid, buf, size, options);
		else
			ret |= report(options,
				      oid, type,
				      FSCK_MSG_GITMODULES_BLOB,
				      "non-blob found at .gitmodules");
		free(buf);
	}


// Source: fsck.c
// Lines 1243-1277
