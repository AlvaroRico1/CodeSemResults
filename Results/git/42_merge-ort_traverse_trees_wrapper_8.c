static int traverse_trees_wrapper(struct index_state *istate,
				  int n,
				  struct tree_desc *t,
				  struct traverse_info *info)
{
	int ret, i, old_offset;
	traverse_callback_t old_fn;
	char *old_callback_data_traverse_path;
	struct merge_options *opt = info->data;
	struct rename_info *renames = &opt->priv->renames;

	assert(renames->dir_rename_mask == 2 || renames->dir_rename_mask == 4);

	old_callback_data_traverse_path = renames->callback_data_traverse_path;
	old_fn = info->fn;
	old_offset = renames->callback_data_nr;

	renames->callback_data_traverse_path = NULL;
	info->fn = traverse_trees_wrapper_callback;
	ret = traverse_trees(istate, n, t, info);
	if (ret < 0)
		return ret;

	info->traverse_path = renames->callback_data_traverse_path;
	info->fn = old_fn;
	for (i = old_offset; i < renames->callback_data_nr; ++i) {
		info->fn(n,
			 renames->callback_data[i].mask,
			 renames->callback_data[i].dirmask,
			 renames->callback_data[i].names,
			 info);
	}

	renames->callback_data_nr = old_offset;
	free(renames->callback_data_traverse_path);
	renames->callback_data_traverse_path = old_callback_data_traverse_path;
	info->traverse_path = NULL;
	return 0;
}


// Source: merge-ort.c
// Lines 748-786
