static struct line_log_data *lookup_line_range(struct rev_info *revs,
					       struct commit *commit)
{
	struct line_log_data *ret = NULL;
	struct line_log_data *d;

	ret = lookup_decoration(&revs->line_log_data, &commit->object);

	for (d = ret; d; d = d->next)
		range_set_check_invariants(&d->ranges);

	return ret;
}


// Source: line-log.c
// Lines 727-739
