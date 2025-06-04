int git_hashsig_create_fromfile(
	git_hashsig **out,
	const char *path,
	git_hashsig_option_t opts)
{
	uint8_t buf[0x1000];
	ssize_t buflen = 0;
	int error = 0, fd;
	hashsig_in_progress prog;
	git_hashsig *sig = hashsig_alloc(opts);
	GIT_ERROR_CHECK_ALLOC(sig);

	if ((fd = git_futils_open_ro(path)) < 0) {
		git__free(sig);
		return fd;
	}

	if ((error = hashsig_in_progress_init(&prog, sig)) < 0) {
		p_close(fd);
		return error;
	}

	while (!error) {
		if ((buflen = p_read(fd, buf, sizeof(buf))) <= 0) {
			if ((error = (int)buflen) < 0)
				git_error_set(GIT_ERROR_OS,
					"read error on '%s' calculating similarity hashes", path);
			break;
		}

		error = hashsig_add_hashes(sig, buf, buflen, &prog);
	}

	p_close(fd);

	if (!error)
		error = hashsig_finalize_hashes(sig);

	if (!error)
		*out = sig;
	else
		git_hashsig_free(sig);

	return error;
}


// Source: hashsig.c
// Lines 272-316
