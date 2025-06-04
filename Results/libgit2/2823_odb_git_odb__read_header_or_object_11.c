int git_odb__read_header_or_object(
	git_odb_object **out, size_t *len_p, git_object_t *type_p,
	git_odb *db, const git_oid *id)
{
	int error = GIT_ENOTFOUND;
	git_odb_object *object;

	GIT_ASSERT_ARG(db);
	GIT_ASSERT_ARG(id);
	GIT_ASSERT_ARG(out);
	GIT_ASSERT_ARG(len_p);
	GIT_ASSERT_ARG(type_p);

	*out = NULL;

	if (git_oid_is_zero(id))
		return error_null_oid(GIT_ENOTFOUND, "cannot read object");

	if ((object = git_cache_get_raw(odb_cache(db), id)) != NULL) {
		*len_p = object->cached.size;
		*type_p = object->cached.type;
		*out = object;
		return 0;
	}

	error = odb_read_header_1(len_p, type_p, db, id, false);

	if (error == GIT_ENOTFOUND && !git_odb_refresh(db))
		error = odb_read_header_1(len_p, type_p, db, id, true);

	if (error == GIT_ENOTFOUND)
		return git_odb__error_notfound("cannot read header for", id, GIT_OID_HEXSZ);

	/* we found the header; return early */
	if (!error)
		return 0;

	if (error == GIT_PASSTHROUGH) {
		/*
		 * no backend has header-reading functionality
		 * so try using `git_odb_read` instead
		 */
		error = git_odb_read(&object, db, id);
		if (!error) {
			*len_p = object->cached.size;
			*type_p = object->cached.type;
			*out = object;
		}
	}

	return error;
}


// Source: odb.c
// Lines 1129-1180
