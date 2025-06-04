static int filesystem_iterator_frame_push(
	filesystem_iterator *iter,
	filesystem_iterator_entry *frame_entry)
{
	filesystem_iterator_frame *new_frame = NULL;
	git_fs_path_diriter diriter = GIT_FS_PATH_DIRITER_INIT;
	git_str root = GIT_STR_INIT;
	const char *path;
	filesystem_iterator_entry *entry;
	struct stat statbuf;
	size_t path_len;
	int error;

	if (iter->frames.size == FILESYSTEM_MAX_DEPTH) {
		git_error_set(GIT_ERROR_REPOSITORY,
			"directory nesting too deep (%"PRIuZ")", iter->frames.size);
		return -1;
	}

	new_frame = git_array_alloc(iter->frames);
	GIT_ERROR_CHECK_ALLOC(new_frame);

	memset(new_frame, 0, sizeof(filesystem_iterator_frame));

	if (frame_entry)
		git_str_joinpath(&root, iter->root, frame_entry->path);
	else
		git_str_puts(&root, iter->root);

	if (git_str_oom(&root) ||
	    git_path_validate_str_length(iter->base.repo, &root) < 0) {
		error = -1;
		goto done;
	}

	new_frame->path_len = frame_entry ? frame_entry->path_len : 0;

	/* Any error here is equivalent to the dir not existing, skip over it */
	if ((error = git_fs_path_diriter_init(
			&diriter, root.ptr, iter->dirload_flags)) < 0) {
		error = GIT_ENOTFOUND;
		goto done;
	}

	if ((error = git_vector_init(&new_frame->entries, 64,
			iterator__ignore_case(&iter->base) ?
			filesystem_iterator_entry_cmp_icase :
			filesystem_iterator_entry_cmp)) < 0)
		goto done;

	if ((error = git_pool_init(&new_frame->entry_pool, 1)) < 0)
		goto done;

	/* check if this directory is ignored */
	filesystem_iterator_frame_push_ignores(iter, frame_entry, new_frame);

	while ((error = git_fs_path_diriter_next(&diriter)) == 0) {
		iterator_pathlist_search_t pathlist_match = ITERATOR_PATHLIST_FULL;
		git_str path_str = GIT_STR_INIT;
		bool dir_expected = false;

		if ((error = git_fs_path_diriter_fullpath(&path, &path_len, &diriter)) < 0)
			goto done;

		path_str.ptr = (char *)path;
		path_str.size = path_len;

		if ((error = git_path_validate_str_length(iter->base.repo, &path_str)) < 0)
			goto done;

		GIT_ASSERT(path_len > iter->root_len);

		/* remove the prefix if requested */
		path += iter->root_len;
		path_len -= iter->root_len;

		/* examine start / end and the pathlist to see if this path is in it.
		 * note that since we haven't yet stat'ed the path, we cannot know
		 * whether it's a directory yet or not, so this can give us an
		 * expected type (S_IFDIR or S_IFREG) that we should examine)
		 */
		if (!filesystem_iterator_examine_path(&dir_expected, &pathlist_match,
			iter, frame_entry, path, path_len))
			continue;

		/* TODO: don't need to stat if assume unchanged for this path and
		 * we have an index, we can just copy the data out of it.
		 */

		if ((error = git_fs_path_diriter_stat(&statbuf, &diriter)) < 0) {
			/* file was removed between readdir and lstat */
			if (error == GIT_ENOTFOUND)
				continue;

			/* treat the file as unreadable */
			memset(&statbuf, 0, sizeof(statbuf));
			statbuf.st_mode = GIT_FILEMODE_UNREADABLE;

			error = 0;
		}

		iter->base.stat_calls++;

		/* Ignore wacky things in the filesystem */
		if (!S_ISDIR(statbuf.st_mode) &&
			!S_ISREG(statbuf.st_mode) &&
			!S_ISLNK(statbuf.st_mode) &&
			statbuf.st_mode != GIT_FILEMODE_UNREADABLE)
			continue;

		if (filesystem_iterator_is_dot_git(iter, path, path_len))
			continue;

		/* convert submodules to GITLINK and remove trailing slashes */
		if (S_ISDIR(statbuf.st_mode)) {
			bool submodule = false;

			if ((error = filesystem_iterator_is_submodule(&submodule,
					iter, path, path_len)) < 0)
				goto done;

			if (submodule)
				statbuf.st_mode = GIT_FILEMODE_COMMIT;
		}


// Source: iterator.c
// Lines 1335-1458
