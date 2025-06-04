mode_tree_search_for(struct mode_tree_data *mtd)
{
	struct mode_tree_item	*mti, *last, *next;

	if (mtd->search == NULL)
		return (NULL);

	mti = last = mtd->line_list[mtd->current].item;
	for (;;) {
		if (!TAILQ_EMPTY(&mti->children))
			mti = TAILQ_FIRST(&mti->children);
		else if ((next = TAILQ_NEXT(mti, entry)) != NULL)
			mti = next;
		else {
			for (;;) {
				mti = mti->parent;
				if (mti == NULL)
					break;
				if ((next = TAILQ_NEXT(mti, entry)) != NULL) {
					mti = next;
					break;
				}
			}
		}
		if (mti == NULL)
			mti = TAILQ_FIRST(&mtd->children);
		if (mti == last)
			break;

		if (mtd->searchcb == NULL) {
			if (strstr(mti->name, mtd->search) != NULL)
				return (mti);
			continue;
		}
		if (mtd->searchcb(mtd->modedata, mti->itemdata, mtd->search))
			return (mti);
	}
	return (NULL);
}


// Source: mode-tree.c
// Lines 790-828
