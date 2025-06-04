int git_mwindow_put_pack(struct git_pack_file *pack)
{
	int count, error;
	struct git_pack_file *pack_to_delete = NULL;

	if ((error = git_mutex_lock(&git__mwindow_mutex)) < 0)
		return error;

	/* put before get would be a corrupted state */
	GIT_ASSERT(git__pack_cache);

	/* if we cannot find it, the state is corrupted */
	GIT_ASSERT(git_strmap_exists(git__pack_cache, pack->pack_name));

	count = git_atomic32_dec(&pack->refcount);
	if (count == 0) {
		git_strmap_delete(git__pack_cache, pack->pack_name);
		pack_to_delete = pack;
	}
	git_mutex_unlock(&git__mwindow_mutex);
	git_packfile_free(pack_to_delete, false);

	return 0;
}


// Source: mwindow.c
// Lines 107-130
