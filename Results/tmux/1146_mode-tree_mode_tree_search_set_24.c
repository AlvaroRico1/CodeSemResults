mode_tree_search_set(struct mode_tree_data *mtd)
{
	struct mode_tree_item	*mti, *loop;
	uint64_t		 tag;

	mti = mode_tree_search_for(mtd);
	if (mti == NULL)
		return;
	tag = mti->tag;

	loop = mti->parent;
	while (loop != NULL) {
		loop->expanded = 1;
		loop = loop->parent;
	}

	mode_tree_build(mtd);
	mode_tree_set_current(mtd, tag);
	mode_tree_draw(mtd);
	mtd->wp->flags |= PANE_REDRAW;
}


// Source: mode-tree.c
// Lines 831-851
