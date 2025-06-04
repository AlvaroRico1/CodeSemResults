static int odb_exists_prefix_1(git_oid *out, git_odb *db,
	const git_oid *key, size_t len, bool only_refreshed)
{
	size_t i;
	int error = GIT_ENOTFOUND, num_found = 0;
	git_oid last_found = {{0}}, found;

	if ((error = git_mutex_lock(&db->lock)) < 0) {
		git_error_set(GIT_ERROR_ODB, "failed to acquire the odb lock");
		return error;
	}
	error = GIT_ENOTFOUND;
	for (i = 0; i < db->backends.length; ++i) {
		backend_internal *internal = git_vector_get(&db->backends, i);
		git_odb_backend *b = internal->backend;

		if (only_refreshed && !b->refresh)
			continue;

		if (!b->exists_prefix)
			continue;

		error = b->exists_prefix(&found, b, key, len);
		if (error == GIT_ENOTFOUND || error == GIT_PASSTHROUGH)
			continue;
		if (error) {
			git_mutex_unlock(&db->lock);
			return error;
		}

		/* make sure found item doesn't introduce ambiguity */
		if (num_found) {
			if (git_oid__cmp(&last_found, &found)) {
				git_mutex_unlock(&db->lock);
				return git_odb__error_ambiguous("multiple matches for prefix");
			}
		} else {
			git_oid_cpy(&last_found, &found);
			num_found++;
		}
	}


// Source: odb.c
// Lines 912-952
