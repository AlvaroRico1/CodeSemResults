static int git_mwindow_find_lru_file_locked(git_mwindow_file **out)
{
	git_mwindow_ctl *ctl = &git_mwindow__mem_ctl;
	git_mwindow_file *lru_file = NULL, *current_file = NULL;
	git_mwindow *lru_window = NULL;
	size_t i;

	git_vector_foreach(&ctl->windowfiles, i, current_file) {
		git_mwindow *mru_window = NULL;
		if (!git_mwindow_scan_recently_used(
				current_file, &mru_window, NULL, true, GIT_MWINDOW__MRU)) {
			continue;
		}
		if (!lru_window || lru_window->last_used > mru_window->last_used) {
			lru_window = mru_window;
			lru_file = current_file;
		}
	}

	if (!lru_file) {
		git_error_set(GIT_ERROR_OS, "failed to close memory window file; couldn't find LRU");
		return -1;
	}

	*out = lru_file;
	return 0;
}


// Source: mwindow.c
// Lines 302-328
