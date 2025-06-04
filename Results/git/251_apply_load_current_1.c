static int load_current(struct apply_state *state,
			struct image *image,
			struct patch *patch)
{
	struct strbuf buf = STRBUF_INIT;
	int status, pos;
	size_t len;
	char *img;
	struct stat st;
	struct cache_entry *ce;
	char *name = patch->new_name;
	unsigned mode = patch->new_mode;

	if (!patch->is_new)
		BUG("patch to %s is not a creation", patch->old_name);

	pos = index_name_pos(state->repo->index, name, strlen(name));
	if (pos < 0)
		return error(_("%s: does not exist in index"), name);
	ce = state->repo->index->cache[pos];
	if (lstat(name, &st)) {
		if (errno != ENOENT)
			return error_errno("%s", name);
		if (checkout_target(state->repo->index, ce, &st))
			return -1;
	}
	if (verify_index_match(state, ce, &st))
		return error(_("%s: does not match index"), name);

	status = load_patch_target(state, &buf, ce, &st, patch, name, mode);
	if (status < 0)
		return status;
	else if (status)
		return -1;
	img = strbuf_detach(&buf, &len);
	prepare_image(image, img, len, !patch->is_binary);
	return 0;
}


// Source: apply.c
// Lines 3531-3568
