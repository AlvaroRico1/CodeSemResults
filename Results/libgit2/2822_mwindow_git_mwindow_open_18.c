unsigned char *git_mwindow_open(
	git_mwindow_file *mwf,
	git_mwindow **cursor,
	off64_t offset,
	size_t extra,
	unsigned int *left)
{
	git_mwindow_ctl *ctl = &git_mwindow__mem_ctl;
	git_mwindow *w = *cursor;

	if (git_mutex_lock(&git__mwindow_mutex)) {
		git_error_set(GIT_ERROR_THREAD, "unable to lock mwindow mutex");
		return NULL;
	}

	if (!w || !(git_mwindow_contains(w, offset) && git_mwindow_contains(w, offset + extra))) {
		if (w) {
			w->inuse_cnt--;
		}

		for (w = mwf->windows; w; w = w->next) {
			if (git_mwindow_contains(w, offset) &&
				git_mwindow_contains(w, offset + extra))
				break;
		}

		/*
		 * If there isn't a suitable window, we need to create a new
		 * one.
		 */
		if (!w) {
			w = new_window_locked(mwf->fd, mwf->size, offset);
			if (w == NULL) {
				git_mutex_unlock(&git__mwindow_mutex);
				return NULL;
			}
			w->next = mwf->windows;
			mwf->windows = w;
		}
	}

	/* If we changed w, store it in the cursor */
	if (w != *cursor) {
		w->last_used = ctl->used_ctr++;
		w->inuse_cnt++;
		*cursor = w;
	}

	offset -= w->offset;

	if (left)
		*left = (unsigned int)(w->window_map.len - offset);

	git_mutex_unlock(&git__mwindow_mutex);
	return (unsigned char *) w->window_map.data + offset;
}


// Source: mwindow.c
// Lines 394-449
