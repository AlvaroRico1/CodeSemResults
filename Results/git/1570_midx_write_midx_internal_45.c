static int write_midx_internal(const char *object_dir,
			       struct string_list *packs_to_drop,
			       const char *preferred_pack_name,
			       unsigned flags)
{
	char *midx_name;
	unsigned char midx_hash[GIT_MAX_RAWSZ];
	uint32_t i;
	struct hashfile *f = NULL;
	struct lock_file lk;
	struct write_midx_context ctx = { 0 };
	struct multi_pack_index *cur;
	int pack_name_concat_len = 0;
	int dropped_packs = 0;
	int result = 0;
	struct chunkfile *cf;

	/* Ensure the given object_dir is local, or a known alternate. */
	find_odb(the_repository, object_dir);

	midx_name = get_midx_filename(object_dir);
	if (safe_create_leading_directories(midx_name))
		die_errno(_("unable to create leading directories of %s"),
			  midx_name);

	for (cur = get_multi_pack_index(the_repository); cur; cur = cur->next) {
		if (!strcmp(object_dir, cur->object_dir)) {
			ctx.m = cur;
			break;
		}
	}

	if (ctx.m && !midx_checksum_valid(ctx.m)) {
		warning(_("ignoring existing multi-pack-index; checksum mismatch"));
		ctx.m = NULL;
	}

	ctx.nr = 0;
	ctx.alloc = ctx.m ? ctx.m->num_packs : 16;
	ctx.info = NULL;
	ALLOC_ARRAY(ctx.info, ctx.alloc);

	if (ctx.m) {
		for (i = 0; i < ctx.m->num_packs; i++) {
			ALLOC_GROW(ctx.info, ctx.nr + 1, ctx.alloc);

			ctx.info[ctx.nr].orig_pack_int_id = i;
			ctx.info[ctx.nr].pack_name = xstrdup(ctx.m->pack_names[i]);
			ctx.info[ctx.nr].p = ctx.m->packs[i];
			ctx.info[ctx.nr].expired = 0;

			if (flags & MIDX_WRITE_REV_INDEX) {
				/*
				 * If generating a reverse index, need to have
				 * packed_git's loaded to compare their
				 * mtimes and object count.
				 */
				if (prepare_midx_pack(the_repository, ctx.m, i)) {
					error(_("could not load pack"));
					result = 1;
					goto cleanup;
				}

				if (open_pack_index(ctx.m->packs[i]))
					die(_("could not open index for %s"),
					    ctx.m->packs[i]->pack_name);
				ctx.info[ctx.nr].p = ctx.m->packs[i];
			}

			ctx.nr++;
		}
	}

	ctx.pack_paths_checked = 0;
	if (flags & MIDX_PROGRESS)
		ctx.progress = start_delayed_progress(_("Adding packfiles to multi-pack-index"), 0);
	else
		ctx.progress = NULL;

	for_each_file_in_pack_dir(object_dir, add_pack_to_midx, &ctx);
	stop_progress(&ctx.progress);

	if (ctx.m && ctx.nr == ctx.m->num_packs && !packs_to_drop) {
		struct bitmap_index *bitmap_git;
		int bitmap_exists;
		int want_bitmap = flags & MIDX_WRITE_BITMAP;

		bitmap_git = prepare_midx_bitmap_git(ctx.m);
		bitmap_exists = bitmap_git && bitmap_is_midx(bitmap_git);
		free_bitmap_index(bitmap_git);

		if (bitmap_exists || !want_bitmap) {
			/*
			 * The correct MIDX already exists, and so does a
			 * corresponding bitmap (or one wasn't requested).
			 */
			if (!want_bitmap)
				clear_midx_files_ext(object_dir, ".bitmap",
						     NULL);
			goto cleanup;
		}
	}


// Source: midx.c
// Lines 1049-1150
