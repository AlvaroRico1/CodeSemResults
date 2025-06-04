static inline int possible_side_renames(struct rename_info *renames,
					unsigned side_index)
{
	return renames->pairs[side_index].nr > 0 &&
	       !strintmap_empty(&renames->relevant_sources[side_index]);
}

static inline int possible_renames(struct rename_info *renames)
{
	return possible_side_renames(renames, 1) ||
	       possible_side_renames(renames, 2) ||
	       !strmap_empty(&renames->cached_pairs[1]) ||
	       !strmap_empty(&renames->cached_pairs[2]);
}


// Source: merge-ort.c
// Lines 2757-2770
