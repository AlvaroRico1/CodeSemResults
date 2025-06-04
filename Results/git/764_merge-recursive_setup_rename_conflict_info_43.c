static inline void setup_rename_conflict_info(enum rename_type rename_type,
					      struct merge_options *opt,
					      struct rename *ren1,
					      struct rename *ren2)
{
	struct rename_conflict_info *ci;

	/*
	 * When we have two renames involved, it's easiest to get the
	 * correct things into stage 2 and 3, and to make sure that the
	 * content merge puts HEAD before the other branch if we just
	 * ensure that branch1 == opt->branch1.  So, simply flip arguments
	 * around if we don't have that.
	 */
	if (ren2 && ren1->branch != opt->branch1) {
		setup_rename_conflict_info(rename_type, opt, ren2, ren1);
		return;
	}

	CALLOC_ARRAY(ci, 1);
	ci->rename_type = rename_type;
	ci->ren1 = ren1;
	ci->ren2 = ren2;

	ci->ren1->dst_entry->processed = 0;
	ci->ren1->dst_entry->rename_conflict_info = ci;
	if (ren2) {
		ci->ren2->dst_entry->rename_conflict_info = ci;
	}
}


// Source: merge-recursive.c
// Lines 281-310
