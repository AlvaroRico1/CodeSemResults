static int collect_diff_cb(long start_a, long count_a,
			   long start_b, long count_b,
			   void *data)
{
	struct collect_diff_cbdata *d = data;

	if (count_a >= 0)
		range_set_append(&d->diff->parent, start_a, start_a + count_a);
	if (count_b >= 0)
		range_set_append(&d->diff->target, start_b, start_b + count_b);

	return 0;
}


// Source: line-log.c
// Lines 315-327
