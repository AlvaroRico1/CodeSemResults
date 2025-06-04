static int traverse_trees_wrapper_callback(int n,
					   unsigned long mask,
					   unsigned long dirmask,
					   struct name_entry *names,
					   struct traverse_info *info)
{
	struct merge_options *opt = info->data;
	struct rename_info *renames = &opt->priv->renames;
	unsigned filemask = mask & ~dirmask;

	assert(n==3);

	if (!renames->callback_data_traverse_path)
		renames->callback_data_traverse_path = xstrdup(info->traverse_path);

	if (filemask && filemask == renames->dir_rename_mask)
		renames->dir_rename_mask = 0x07;

	ALLOC_GROW(renames->callback_data, renames->callback_data_nr + 1,
		   renames->callback_data_alloc);
	renames->callback_data[renames->callback_data_nr].mask = mask;
	renames->callback_data[renames->callback_data_nr].dirmask = dirmask;
	COPY_ARRAY(renames->callback_data[renames->callback_data_nr].names,
		   names, 3);
	renames->callback_data_nr++;

	return mask;
}


// Source: merge-ort.c
// Lines 712-739
