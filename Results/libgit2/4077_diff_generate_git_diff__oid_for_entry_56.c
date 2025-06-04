int git_diff__oid_for_entry(
	git_oid *out,
	git_diff *d,
	const git_index_entry *src,
	uint16_t mode,
	const git_oid *update_match)
{
	git_diff_generated *diff;
	git_str full_path = GIT_STR_INIT;
	git_index_entry entry = *src;
	git_filter_list *fl = NULL;
	int error = 0;

	GIT_ASSERT(d->type == GIT_DIFF_TYPE_GENERATED);
	diff = (git_diff_generated *)d;

	memset(out, 0, sizeof(*out));

	if (git_repository_workdir_path(&full_path, diff->base.repo, entry.path) < 0)
		return -1;

	if (!mode) {
		struct stat st;

		diff->base.perf.stat_calls++;

		if (p_stat(full_path.ptr, &st) < 0) {
			error = git_fs_path_set_error(errno, entry.path, "stat");
			git_str_dispose(&full_path);
			return error;
		}

		git_index_entry__init_from_stat(&entry,
			&st, (diff->diffcaps & GIT_DIFFCAPS_TRUST_MODE_BITS) != 0);
	}

	/* calculate OID for file if possible */
	if (S_ISGITLINK(mode)) {
		git_submodule *sm;

		if (!git_submodule_lookup(&sm, diff->base.repo, entry.path)) {
			const git_oid *sm_oid = git_submodule_wd_id(sm);
			if (sm_oid)
				git_oid_cpy(out, sm_oid);
			git_submodule_free(sm);
		} else {
			/* if submodule lookup failed probably just in an intermediate
			 * state where some init hasn't happened, so ignore the error
			 */
			git_error_clear();
		}
	} else if (S_ISLNK(mode)) {
		error = git_odb__hashlink(out, full_path.ptr);
		diff->base.perf.oid_calculations++;
	} else if (!git__is_sizet(entry.file_size)) {
		git_error_set(GIT_ERROR_NOMEMORY, "file size overflow (for 32-bits) on '%s'",
			entry.path);
		error = -1;
	} else if (!(error = git_filter_list_load(&fl,
		diff->base.repo, NULL, entry.path,
		GIT_FILTER_TO_ODB, GIT_FILTER_ALLOW_UNSAFE)))
	{
		int fd = git_futils_open_ro(full_path.ptr);
		if (fd < 0)
			error = fd;
		else {
			error = git_odb__hashfd_filtered(
				out, fd, (size_t)entry.file_size, GIT_OBJECT_BLOB, fl);
			p_close(fd);
			diff->base.perf.oid_calculations++;
		}

		git_filter_list_free(fl);
	}

	/* update index for entry if requested */
	if (!error && update_match && git_oid_equal(out, update_match)) {
		git_index *idx;
		git_index_entry updated_entry;

		memcpy(&updated_entry, &entry, sizeof(git_index_entry));
		updated_entry.mode = mode;
		git_oid_cpy(&updated_entry.id, out);

		if (!(error = git_repository_index__weakptr(&idx,
			diff->base.repo))) {
			error = git_index_add(idx, &updated_entry);
			diff->index_updated = true;
		}
 	}


// Source: diff_generate.c
// Lines 605-694
