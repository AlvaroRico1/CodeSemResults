static void iterate_over_patch(git_patch *patch, diff_expects *exp)
{
	size_t h, num_h = git_patch_num_hunks(patch), num_l;

	exp->files++;
	exp->hunks += (int)num_h;

	/* let's iterate in reverse, just because we can! */
	for (h = 1, num_l = 0; h <= num_h; ++h)
		num_l += git_patch_num_lines_in_hunk(patch, num_h - h);

	exp->lines += (int)num_l;
}


// Source: diffiter.c
// Lines 252-264
