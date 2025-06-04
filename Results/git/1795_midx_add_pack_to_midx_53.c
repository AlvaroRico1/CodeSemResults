static void add_pack_to_midx(const char *full_path, size_t full_path_len,
			     const char *file_name, void *data)
{
	struct write_midx_context *ctx = data;

	if (ends_with(file_name, ".idx")) {
		display_progress(ctx->progress, ++ctx->pack_paths_checked);
		if (ctx->m && midx_contains_pack(ctx->m, file_name))
			return;

		ALLOC_GROW(ctx->info, ctx->nr + 1, ctx->alloc);

		ctx->info[ctx->nr].p = add_packed_git(full_path,
						      full_path_len,
						      0);

		if (!ctx->info[ctx->nr].p) {
			warning(_("failed to add packfile '%s'"),
				full_path);
			return;
		}

		if (open_pack_index(ctx->info[ctx->nr].p)) {
			warning(_("failed to open pack-index '%s'"),
				full_path);
			close_pack(ctx->info[ctx->nr].p);
			FREE_AND_NULL(ctx->info[ctx->nr].p);
			return;
		}

		ctx->info[ctx->nr].pack_name = xstrdup(file_name);
		ctx->info[ctx->nr].orig_pack_int_id = ctx->nr;
		ctx->info[ctx->nr].expired = 0;
		ctx->nr++;
	}
}


// Source: midx.c
// Lines 465-500
