static void cache_new_pair(struct rename_info *renames,
			   int side,
			   char *old_path,
			   char *new_path,
			   int free_old_value)
{
	char *old_value;
	new_path = xstrdup(new_path);
	old_value = strmap_put(&renames->cached_pairs[side],
			       old_path, new_path);
	strset_add(&renames->cached_target_names[side], new_path);
	if (free_old_value)
		free(old_value);
	else
		assert(!old_value);
}


// Source: merge-ort.c
// Lines 2851-2866
