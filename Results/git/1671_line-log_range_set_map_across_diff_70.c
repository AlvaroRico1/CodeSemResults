static void range_set_map_across_diff(struct range_set *out,
				      struct range_set *rs,
				      struct diff_ranges *diff,
				      struct diff_ranges **touched_out)
{
	struct diff_ranges *touched = xmalloc(sizeof(*touched));
	struct range_set tmp1 = RANGE_SET_INIT;
	struct range_set tmp2 = RANGE_SET_INIT;

	diff_ranges_init(touched);
	diff_ranges_filter_touched(touched, diff, rs);
	range_set_difference(&tmp1, rs, &touched->target);
	range_set_shift_diff(&tmp2, &tmp1, diff);
	range_set_union(out, &tmp2, &touched->parent);
	range_set_release(&tmp1);
	range_set_release(&tmp2);

	*touched_out = touched;
}


// Source: line-log.c
// Lines 453-471
