void git_mwindow_close(git_mwindow **window)
{
	git_mwindow *w = *window;
	if (w) {
		if (git_mutex_lock(&git__mwindow_mutex)) {
			git_error_set(GIT_ERROR_THREAD, "unable to lock mwindow mutex");
			return;
		}

		w->inuse_cnt--;
		git_mutex_unlock(&git__mwindow_mutex);
		*window = NULL;
	}
}


// Source: mwindow.c
// Lines 528-541
