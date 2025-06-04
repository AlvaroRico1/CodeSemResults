mode_tree_menu_callback(__unused struct menu *menu, __unused u_int idx,
    key_code key, void *data)
{
	struct mode_tree_menu		*mtm = data;
	struct mode_tree_data		*mtd = mtm->data;
	struct mode_tree_item		*mti;

	if (mtd->dead || key == KEYC_NONE)
		goto out;

	if (mtm->line >= mtd->line_size)
		goto out;
	mti = mtd->line_list[mtm->line].item;
	if (mti->itemdata != mtm->itemdata)
		goto out;
	mtd->current = mtm->line;
	mtd->menucb(mtd->modedata, mtm->c, key);

out:
	mode_tree_remove_ref(mtd);
	free(mtm);
}


// Source: mode-tree.c
// Lines 909-930
