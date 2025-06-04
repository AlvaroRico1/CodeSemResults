int git_midx_writer_new(
		git_midx_writer **out,
		const char *pack_dir)
{
	git_midx_writer *w = git__calloc(1, sizeof(git_midx_writer));
	GIT_ERROR_CHECK_ALLOC(w);

	if (git_str_sets(&w->pack_dir, pack_dir) < 0) {
		git__free(w);
		return -1;
	}
	git_fs_path_squash_slashes(&w->pack_dir);

	if (git_vector_init(&w->packs, 0, packfile__cmp) < 0) {
		git_str_dispose(&w->pack_dir);
		git__free(w);
		return -1;
	}

	*out = w;
	return 0;
}


// Source: midx.c
// Lines 500-521
