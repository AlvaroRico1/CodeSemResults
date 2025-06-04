static int read_prefix_1(git_odb_object **out, git_odb *db,
		const git_oid *key, size_t len, bool only_refreshed)
{
	size_t i;
	int error = 0;
	git_oid found_full_oid = {{0}};
	git_rawobj raw = {0};
	void *data = NULL;
	bool found = false;
	git_odb_object *object;

	if ((error = git_mutex_lock(&db->lock)) < 0) {
		git_error_set(GIT_ERROR_ODB, "failed to acquire the odb lock");
		return error;
	}
	for (i = 0; i < db->backends.length; ++i) {
		backend_internal *internal = git_vector_get(&db->backends, i);
		git_odb_backend *b = internal->backend;

		if (only_refreshed && !b->refresh)
			continue;

		if (b->read_prefix != NULL) {
			git_oid full_oid;
			error = b->read_prefix(&full_oid, &raw.data, &raw.len, &raw.type, b, key, len);

			if (error == GIT_ENOTFOUND || error == GIT_PASSTHROUGH) {
				error = 0;
				continue;
			}

			if (error) {
				git_mutex_unlock(&db->lock);
				goto out;
			}

			git__free(data);
			data = raw.data;

			if (found && git_oid__cmp(&full_oid, &found_full_oid)) {
				git_str buf = GIT_STR_INIT;

				git_str_printf(&buf, "multiple matches for prefix: %s",
					git_oid_tostr_s(&full_oid));
				git_str_printf(&buf, " %s",
					git_oid_tostr_s(&found_full_oid));

				error = git_odb__error_ambiguous(buf.ptr);
				git_str_dispose(&buf);
				git_mutex_unlock(&db->lock);
				goto out;
			}

			found_full_oid = full_oid;
			found = true;
		}
	}
	git_mutex_unlock(&db->lock);

	if (!found)
		return GIT_ENOTFOUND;

	if (git_odb__strict_hash_verification) {
		git_oid hash;

		if ((error = git_odb_hash(&hash, raw.data, raw.len, raw.type)) < 0)
			goto out;

		if (!git_oid_equal(&found_full_oid, &hash)) {
			error = git_odb__error_mismatch(&found_full_oid, &hash);
			goto out;
		}
	}

	if ((object = odb_object__alloc(&found_full_oid, &raw)) == NULL) {
		error = -1;
		goto out;
	}

	*out = git_cache_store_raw(odb_cache(db), object);

out:
	if (error)
		git__free(raw.data);

	return error;
}


// Source: odb.c
// Lines 1303-1389
