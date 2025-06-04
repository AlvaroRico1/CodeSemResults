void shift_tree_by(struct repository *r,
		   const struct object_id *hash1,
		   const struct object_id *hash2,
		   struct object_id *shifted,
		   const char *shift_prefix)
{
	struct object_id sub1, sub2;
	unsigned short mode1, mode2;
	unsigned candidate = 0;

	/* Can hash2 be a tree at shift_prefix in tree hash1? */
	if (!get_tree_entry(r, hash1, shift_prefix, &sub1, &mode1) &&
	    S_ISDIR(mode1))
		candidate |= 1;

	/* Can hash1 be a tree at shift_prefix in tree hash2? */
	if (!get_tree_entry(r, hash2, shift_prefix, &sub2, &mode2) &&
	    S_ISDIR(mode2))
		candidate |= 2;

	if (candidate == 3) {
		/* Both are plausible -- we need to evaluate the score */
		int best_score = score_trees(hash1, hash2);
		int score;

		candidate = 0;
		score = score_trees(&sub1, hash2);
		if (score > best_score) {
			candidate = 1;
			best_score = score;
		}
		score = score_trees(&sub2, hash1);
		if (score > best_score)
			candidate = 2;
	}

	if (!candidate) {
		/* Neither is plausible -- do not shift */
		oidcpy(shifted, hash2);
		return;
	}

	if (candidate == 1)
		/*
		 * shift tree2 down by adding shift_prefix above it
		 * to match tree1.
		 */
		splice_tree(hash1, shift_prefix, hash2, shifted);
	else
		/*
		 * shift tree2 up by removing shift_prefix from it
		 * to match tree1.
		 */
		oidcpy(shifted, &sub2);
}


// Source: match-trees.c
// Lines 311-365
