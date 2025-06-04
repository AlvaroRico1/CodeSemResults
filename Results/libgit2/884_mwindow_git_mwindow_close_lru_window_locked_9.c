static int git_mwindow_close_lru_window_locked(void)
{
	git_mwindow_ctl *ctl = &git_mwindow__mem_ctl;
	git_mwindow_file *cur;
	size_t i;
	git_mwindow *lru_window = NULL, *lru_last = NULL, **list = NULL;

	git_vector_foreach(&ctl->windowfiles, i, cur) {
		if (git_mwindow_scan_recently_used(
				cur, &lru_window, &lru_last, false, GIT_MWINDOW__LRU)) {
			list = &cur->windows;
		}
	}

	if (!lru_window) {
		git_error_set(GIT_ERROR_OS, "failed to close memory window; couldn't find LRU");
		return -1;
	}

	ctl->mapped -= lru_window->window_map.len;
	git_futils_mmap_free(&lru_window->window_map);

	if (lru_last)
		lru_last->next = lru_window->next;
	else
		*list = lru_window->next;

	git__free(lru_window);
	ctl->open_windows--;

	return 0;
}


// Source: mwindow.c
// Lines 262-293
