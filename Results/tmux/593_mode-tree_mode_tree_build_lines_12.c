mode_tree_build_lines(struct mode_tree_data *mtd,
    struct mode_tree_list *mtl, u_int depth)
{
	struct mode_tree_item	*mti;
	struct mode_tree_line	*line;
	u_int			 i;
	int			 flat = 1;

	mtd->depth = depth;
	TAILQ_FOREACH(mti, mtl, entry) {
		mtd->line_list = xreallocarray(mtd->line_list,
		    mtd->line_size + 1, sizeof *mtd->line_list);

		line = &mtd->line_list[mtd->line_size++];
		line->item = mti;
		line->depth = depth;
		line->last = (mti == TAILQ_LAST(mtl, mode_tree_list));

		mti->line = (mtd->line_size - 1);
		if (!TAILQ_EMPTY(&mti->children))
			flat = 0;
		if (mti->expanded)
			mode_tree_build_lines(mtd, &mti->children, depth + 1);

		if (mtd->keycb != NULL) {
			mti->key = mtd->keycb(mtd->modedata, mti->itemdata,
			    mti->line);
			if (mti->key == KEYC_UNKNOWN)
				mti->key = KEYC_NONE;
		} else if (mti->line < 10)
			mti->key = '0' + mti->line;
		else if (mti->line < 36)
			mti->key = KEYC_META|('a' + mti->line - 10);
		else
			mti->key = KEYC_NONE;
		if (mti->key != KEYC_NONE) {
			mti->keystr = xstrdup(key_string_lookup_key(mti->key,
			    0));
			mti->keylen = strlen(mti->keystr);
		} else {
			mti->keystr = NULL;
			mti->keylen = 0;
		}
	}
	TAILQ_FOREACH(mti, mtl, entry) {
		for (i = 0; i < mtd->line_size; i++) {
			line = &mtd->line_list[i];
			if (line->item == mti)
				line->flat = flat;
		}
	}
}


// Source: mode-tree.c
// Lines 179-230
