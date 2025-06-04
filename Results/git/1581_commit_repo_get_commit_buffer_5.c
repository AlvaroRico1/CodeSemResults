const void *repo_get_commit_buffer(struct repository *r,
				   const struct commit *commit,
				   unsigned long *sizep)
{
	const void *ret = get_cached_commit_buffer(r, commit, sizep);
	if (!ret) {
		enum object_type type;
		unsigned long size;
		ret = repo_read_object_file(r, &commit->object.oid, &type, &size);
		if (!ret)
			die("cannot read commit object %s",
			    oid_to_hex(&commit->object.oid));
		if (type != OBJ_COMMIT)
			die("expected commit for %s, got %s",
			    oid_to_hex(&commit->object.oid), type_name(type));
		if (sizep)
			*sizep = size;
	}
	return ret;
}


// Source: commit.c
// Lines 292-311
